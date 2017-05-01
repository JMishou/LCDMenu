#include "LCDMenu.h"

LCDMenu::LCDMenu(uint8_t LCDAddress, uint8_t LCDChars, uint8_t LCDRows, Rotary * encoder, OneButton * button, int menuTimeoutMS)
{
  _LCDAddress = LCDAddress;
  _LCDChars = LCDChars;
  _LCDRows = LCDRows;
  _encoder = encoder;
  _button = button;
  _menuTimeoutPeriod = menuTimeoutMS;
}


  void LCDMenu::setup(){
    //Serial.println("setup");

    //LiquidCrystal_I2C lcd(_LCDAddress,_LCDChars, _LCDRows);  // Set the LCD I2C address
    _LCD = new LiquidCrystal_I2C(_LCDAddress,_LCDChars, _LCDRows);  // Set the LCD I2C address
    _LCD->begin();
    //_LCD->backlight();
    _suspendMenu = false;
    _decInput = false;
    _liveDisp = false;
    _lastInput = _lastMillis = millis();
  }


void LCDMenu::printMenu(){


  //Serial.println("Print Menu: ");
    uint8_t i = 0;
    menuItem* currEntry = firstEntry();
    _LCD->clear();
    while (currEntry && i < _LCDRows){
      if (currEntry == _selectedEntry){
        _LCD->setCursor(0, i);
        _LCD->write(0x7E);
        _LCD->setCursor(1,i);
        _LCD->print( currEntry->getMenuText());
      } else {
        _LCD->setCursor(1,i);
        _LCD->print( currEntry->getMenuText());
      }
      if (currEntry->getChild()){
        _LCD->write(0x00);
      }
      currEntry = currEntry->getNextSibling();
      i++;
    }
}


//returns true if menu is active false if timed out
bool LCDMenu::poll(){
  int menuAction = 0;

  unsigned char result = _encoder->process();
    _button->tick();
    if (result) {
      menuAction = (result == DIR_CW ? 1 : -1);
    }
    else if (_buttonPressed){
      menuAction = 1+_buttonPressed;
      _buttonPressed = 0;
    }
    else if (_liveDisp && millis()-_lastMillis > _liveTimeout){
      _lastMillis = millis();
      menuAction = 4;
    }
    if (!_suspendMenu){
      switch( menuAction )
      {
        case -1:
          //Serial.println("Up");
          DoMenuAction(MENU_ACTION_UP);
          break;
        case 1:
          //Serial.println("Down");
          DoMenuAction(MENU_ACTION_DOWN);
          break;
        case 2:
          //Serial.println("Select");
          DoMenuAction( MENU_ACTION_SELECT );
          break;
        case 3:
          //Serial.println("Back");
          DoMenuAction(MENU_ACTION_BACK);
          break;
        case 4:
          //Serial.println("Timeout");
          //DoMenuAction(MENU_ACTION_TIME);
        default:
          break;
      }
    }else{
      //Serial.println("Suspended Menu");
          switch( menuAction )
          {
            case -1:
              if (_decInput){
                //Serial.println("intIncrease");
                intIncrease();
                printInput();
              }
              break;
            case 1:
              if (_decInput){
                //Serial.println("intDecrease");
                intDecrease();
                printInput();
              }
              break;
            case 2:
              //Serial.println("Select");
              if (_decInput && _userInputCallback != NULL){
                _userInputCallback(_intPtr);
                _userInputCallback = NULL;
              }
              _decInput = false;
              _suspendMenu = false;
              _liveDisp = false;
              printMenu();
              break;
            case 3:
              //Serial.println("Back");
              DoMenuAction(MENU_ACTION_BACK);
              break;
            case 4:
                drawInputRow(_liveText);
              break;
            default:
              break;
           }
   }
   if (menuAction == 0 || menuAction == 4){
     if (millis()-_lastInput > _menuTimeoutPeriod  & !_decInput & !_liveDisp){
       _menuTimeout = true;
       SelectRoot();
     }
   } else {
       _lastInput = millis();
       _menuTimeout = false;
   }
   return !_menuTimeout;
}

menuItem * LCDMenu::firstEntry(){
  uint8_t cnt = 0;
  menuItem* curr = _selectedEntry;
  while (curr){
    curr = curr->getNextSibling();
    cnt++;
  }
  curr = _selectedEntry;

  while (cnt < _LCDRows && curr->getPrevSibling()){
    curr = curr->getPrevSibling();
    cnt++;
  }
  return curr;
}

void LCDMenu::addMenuRoot( menuItem * root)
{
  _rootEntry = _selectedEntry = root;

}

LiquidCrystal_I2C * LCDMenu::getLCD()
{
  return _LCD;
}

void LCDMenu::buttonClick(){
  _buttonPressed = 1;
  printMenu();
}
void LCDMenu::buttonDblClick(){
  _buttonPressed = 2;
}


void LCDMenu::DoMenuAction( MENU_ACTION action )
{
    //Serial.print("Clear");
    _LCD->clear();
    delay(10);
    //PrintMenu();
    switch (action )
    {
      case MENU_ACTION_UP:
      //Serial.println("MENU_ACTION_UP");
        MenuUp();
        break;
      case MENU_ACTION_DOWN:
      //Serial.println("MENU_ACTION_DOWN");
        MenuDown();
        break;
      case MENU_ACTION_SELECT:
      //Serial.println("MENU_ACTION_SELECT");
        MenuSelect();
        break;
      case MENU_ACTION_BACK:
      //Serial.println("MENU_ACTION_BACK");
        MenuBack();
        break;
      default:
        break;
    }
  }


void LCDMenu::MenuUp()
{
  menuItem *prev = _selectedEntry->getPrevSibling();
  if( prev != NULL )
  {
    _selectedEntry = prev;
  }
  else
  {
    //Flash?
  }
  printMenu();
}

void LCDMenu::MenuDown()
{
  menuItem *next = _selectedEntry->getNextSibling();
  if( next != NULL )
  {
    _selectedEntry = next;
  }
  else
  {
    //Flash?
  }
  printMenu();

}

void LCDMenu::MenuSelect()
{
  menuItem *child = _selectedEntry->getChild();
  if( child != NULL )
  {
    _selectedEntry = child;
    printMenu();
  }
  else
  {
     _selectedEntry->ExecuteCallback();
  }
}

void LCDMenu::MenuBack()
{
  if( _selectedEntry->getParent() )
  {
    _selectedEntry = _selectedEntry->getParent();
  }
  printMenu();
}

void LCDMenu::printPage( char* pString[], int nLines )
{
  _suspendMenu = true;
  //Serial.println("PrintPage");
  _LCD->clear();
  for( int i =0; i < nLines; i++ )
  {
    {
       _LCD->setCursor(0, i);
       _LCD->print( pString[i] );
    }
  }
}

void LCDMenu::getInput( int iMin, int iMax, int iStart, int iSteps, char* label[], uint8_t iLabelLines, int32_t * pInt, uint8_t decPlaces, USER_INPUT_CALLBACK userCB )
{
   //Serial.println("DoInput");
  _suspendMenu = true;
  _decInput = true;
  _intPtr = pInt;
  *_intPtr = iStart;
  _intStep = iSteps;
  _intMin = iMin;
  _intMax = iMax;
  _inDecPrec = decPlaces;
  _userInputCallback = userCB;
  //print the label
  printPage( label, iLabelLines );
  printInput();
}

void LCDMenu::printInput(){
   if (_inDecPrec > 0 ){
   dtostrf( ((float)*_intPtr/(float)pow(10,_inDecPrec)), 1, _inDecPrec, _inputBuffer );
  }else{
      itoa( *_intPtr, _inputBuffer, 10 );
  }
  drawInputRow( _inputBuffer );
}

void LCDMenu::setLiveDisp(char * str, int updateRate){
  _liveDisp = true;
  _liveText = str;
  _liveTimeout = updateRate;
  drawInputRow( str );
}

void LCDMenu::intIncrease()
{
  //This function may have bugs when m_max is near the MAXINT limit
  //but if your UI requires users to input MAXINT numbers, you have bigger problems.
  if( *_intPtr + _intStep <= _intMax )
  {
    *_intPtr += _intStep;
  }else{
    *_intPtr = _intMax;
  }
}

void LCDMenu::intDecrease()
{
  //Serial.println((*_intPtr - _intStep) >= _intMin);
  if( (*_intPtr - _intStep) >= _intMin )
  {
    *_intPtr  -= _intStep;
  }else{
    *_intPtr  = _intMin;
  }

}
int LCDMenu::getInputInt()
{
  return  *_intPtr;
}

float LCDMenu::getInputFloat()
{
  return  float(*_intPtr)/pow(10,_inDecPrec);
}

void LCDMenu::drawInputRow( char *pString )
{
  printLineRight( pString, _LCDRows - 1 );
}

void LCDMenu::printLineRight( char* pString, int iRow )
{
  //clear the line
  char buff[ _LCDChars ];
  for( int i = 0; i < _LCDChars; ++i )
  {
    buff[i] = ' ';
  }
    buff[_LCDChars-1] = '\0';

  _LCD->setCursor( 0, iRow );
  _LCD->print( buff );
  //now print the new number
  _LCD->setCursor(_LCDChars - strlen(pString),iRow);
  _LCD->print( pString );
}

void LCDMenu::printLine( char* pString, int iRow )
{
  //clear the line
  _LCD->setCursor( 0, iRow );
  _LCD->print( pString );
}

void LCDMenu::clearLCD()
{
  _LCD->clear();
}

void LCDMenu::pauseMenu(){
  _suspendMenu = true;
}

void LCDMenu::SelectRoot()
{
  _selectedEntry = _rootEntry;
}

void LCDMenu::updatePos(char* val, uint8_t cursorPos, uint8_t line, uint8_t length){
    _LCD->setCursor( cursorPos, line );
    if (strlen(val) > length){
      for(int x = 0; x < length; x++){
        _LCD->setCursor( cursorPos+x, line );
        _LCD->write(val[x]);

      }
    }else{
      if (strlen(val) < length){
        for(int x = 0; x < (length-strlen(val)); x++){
          _LCD->setCursor( cursorPos+x, line );
          _LCD->write(' ');
        }
    }
      _LCD->print( val );
    }
}
