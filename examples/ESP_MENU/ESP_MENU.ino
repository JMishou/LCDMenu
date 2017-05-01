/*
 Original Code by David Andrews
 Butchered and grilled by Jason Mishou
 Arduino_LCD_Menu Library

Licensed under the follwing license:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution.
The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define LCDAddr 0x3F
#define LCDRows 4
#define LCDChars 20
#define EncoderA D7
#define EncoderB D6
#define ButtonPin D1


//#include <LiquidCrystal_I2C.h>
//#include <Rotary.h>
//#include <OneButton.h>
#include "LCDMenu.h"
#include <Wire.h>

//This example is a Stopwatch and Timer.  Although it is mostly functional, it might not be the best
// user interface.  The layout was created more to provide examples of a stopwatch/timer.


Rotary encoder(EncoderA,EncoderB);
OneButton button(ButtonPin, true);
LiquidCrystal_I2C * _lcd;
//Now create the MenuLCD and MenuManager classes.
LCDMenu pMenu(LCDAddr, LCDChars, LCDRows, &encoder, &button);
byte retarrow[8] = {  0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4};
int one = 1;
int zero = 0;
unsigned long lastUpdate = 0;
int updateinterval = 250;
bool menuTimeout = false;



//Global variables used by the sample application
//when the display is showing user results (e.g. time elapsed), the next "select" should send it back into the menu.
unsigned int g_isDisplaying = false;
int g_timerTime = 20;
char g_timeElapsed[LCDChars];
long g_timerRunning = false;
long g_timerTarget = 0;
long g_autoReset = false;
long g_stopMillis = 0;
long g_startMillis = 0;
char liveBuffer[20];

//end Global variables section

//setupMenus
//This function is called during setup to populate the menu with the tree of nodes
//This can be a bit brain-bending to write.  If you draw a tree you want for your menu first
// this code can be a little easier to write.
//
//  First build an entry for each node that will have children.
//
//  This sample code is a simple stopwatch.  Our menu will look like this:
//  Stopwatch
//  |-Start
//  |-Stop
//  |-Reset
//  |-Back
//  Timer
//  |-Set Time
//  |-AutoReset
//  | |-On
//  | |-Off
//  |-Start
//  |-Stop
//  Credits
//  Show Smiley
//
//  The nodes that will have children are:
//  Stopwatch
//  Timer
//  AutoReset

void setupMenus()
{

 Serial.println("Setup Menus");

 //Initialize the LCD Menu
 //g_menuLCD.setup();

  //Add button handlers
  button.attachClick(buttonClick);
  button.attachDoubleClick(buttonDblClick);

  //Create a menuItem for each parent node
  //Stopwatch will be the first entry and therfor will be the root node.
  menuItem * hygrometer = new menuItem("HYGROMETER", NULL, NULL);
  menuItem * waterLevel = new menuItem("WATER LEVEL SENSOR", NULL, NULL );
  menuItem * reservoir = new menuItem("RESERVOIR", NULL, NULL );

  //Add to stopwatch its siblings (i.e. menu entries on the same level)
  hygrometer->addSibling(waterLevel);
  hygrometer->addSibling(reservoir);

  //Add to stopwatch each of its child entries
  hygrometer->addChild( new menuItem("SET LOW MOISTURE", (void *)(&zero), hygrometerCallback) );
  hygrometer->addChild( new menuItem("SET HIGH MOISTURE", (void *)(&one), hygrometerCallback ) );
  hygrometer->addChild( new menuItem("Back", (void *) &pMenu, menuItem_BackCallbackFunc) );

  waterLevel->addChild( new menuItem("EMPTY RESERVOIR", (void *)(&zero), waterLevelCallback) );
  waterLevel->addChild( new menuItem("FULL RESERVOIR", (void *)(&one), waterLevelCallback ) );
  waterLevel->addChild( new menuItem("Back", (void *) &pMenu, menuItem_BackCallbackFunc) );

  reservoir->addChild( new menuItem("SET HEIGHT", NULL, heightCallback) );
  reservoir->addChild( new menuItem("SET VOLUME", NULL, volumeCallback ) );
  reservoir->addChild( new menuItem("Back", (void *) &pMenu, menuItem_BackCallbackFunc) );

  //Set the root entry
  pMenu.addMenuRoot(hygrometer);

  //Create LCD custom chars
  //LCD.createChar( 0, g_smiley );  //Smiley face
  //LCD.createChar( 1, g_frown );   //Frowney face

  //Make sure the menu is drawn correctly after all changes are done
  //g_menuLCD.SelectRoot();
  //g_menuLCD.PrintMenu();

}


void setup()
{
  Wire.begin(D4,D5);
  Serial.begin(115200);
  Serial.println("Ready.");
  pinMode(ButtonPin,INPUT_PULLUP);
  pMenu.setup();
  _lcd = pMenu.getLCD();
  _lcd->createChar( 0, retarrow );
  //_lcd->print( "Building Menus" );
  //Setup the Menus
  setupMenus();
  pMenu.printMenu();
}



void loop()
{
  //Poll the status of the LCDMenu
  if (pMenu.poll()){
    if (menuTimeout){
       //pMenu.printMenu();
    }
    menuTimeout = false;
    if(millis()-g_startMillis > 150){
        g_startMillis = millis();
        dtostrf( ((float)g_startMillis/1000.0), 1, 2, liveBuffer );  
    }
  }else{
    if (!menuTimeout){
      menuTimeout = true;
      //_lcd->noBacklight();
      printDisplay();
    }
    if (millis()-lastUpdate > updateinterval){
      updateDisplay();
      lastUpdate = millis();
    }
    
  }

}

void printDisplay(){
  char *page[3] = { " TEMP XXX F RH XX% ", "    BARO X.XXkPa    ", " FL XX.XL  MC XXX% " };
  page[0][9] = (char)223;
  pMenu.printPage( page, 3);
}

void updateDisplay(){
  char temp[3];
  char hum[2];
  char bar[4];
  char fluid[4];
  char moist[3];

  int _temp = random(-40,120);
  int _hum = random(1,100);
  float _bar = (float)random(0,9)+(float)random(0,99)/100.0;
  float _fluid = (float)random(0,5)+(float)random(0,99)/100.0;
  int _moist = random(1,100);

  itoa(_temp,temp,10);
  itoa(_hum,hum,10);
  dtostrf( _bar, 1, 2, bar ); 
  dtostrf( _fluid, 1, 1, fluid ); 
  itoa(_moist,moist,10);

  pMenu.updatePos(temp,6,0,3);
  pMenu.updatePos(hum,15,0,2);
  pMenu.updatePos(bar,9,1,4);
  pMenu.updatePos(fluid,4,2,4);
  pMenu.updatePos(moist,14,2,3);
}

void hygrometerCallback( char* pMenuText, void *pUserData ){
  Serial.println("hygrometerCallback");
  int high = *((int *)pUserData);
  _lcd->clear();
  if (high){
    Serial.println("SET HIGH MOISTURE");
    _lcd->print("SET HIGH MOISTURE");
  } else {
    Serial.println("SET LOW MOISTURE");
    _lcd->print("SET LOW MOISTURE");
  }
  pMenu.pauseMenu();
}

void waterLevelCallback( char* pMenuText, void *pUserData ){

  int full = *((int *)pUserData);
  _lcd->clear();
  if (full){
    Serial.println("FULL RESERVOIR");
    _lcd->print("FULL RESERVOIR");
  } else {
    Serial.println("EMPTY RESERVOIR");
    _lcd->print("EMPTY RESERVOIR");
  }
  pMenu.pauseMenu();
}

void heightCallback( char* pMenuText, void *pUserData ){

  _lcd->clear();
  _lcd->print("Set Height");
  Serial.println("Set Height");
  char *page[3] = { " TEMP XX.XF  RH XX% ", "    BARO X.XX kPa   ", " FL XX.XL  MC XX.X% " };
  pMenu.printPage( page, 3);
  pMenu.setLiveDisp( liveBuffer, 2000);
  pMenu.pauseMenu();
}

void volumeCallback( char* pMenuText, void *pUserData ){

  //_lcd->clear();
  Serial.println("Set Volume");
  char *pLabel = "Set Value";
  int iNumLabelLines = 1;
  int iMin = 0;
  int iMax = 10000;
  int iStart = 600;
  //Each user input action (such as a turn of rotary enocoder or push of button
  //will step this amount
  int iStep = 50;
  
  pMenu.getInput( iMin, iMax, iStart, iStep, &pLabel, iNumLabelLines, &g_timerTime, 2 );
  
}


//Here we set the button input to execute the menu function button click
void buttonClick(){
Serial.println("Click");
  pMenu.buttonClick();
}

//Here we set the button input to execute the menu function button double click
void buttonDblClick(){
  Serial.println("Double Click");
  pMenu.buttonDblClick();
}
