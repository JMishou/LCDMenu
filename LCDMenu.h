#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Rotary.h>
#include <OneButton.h>
#include <stdio.h>
#include "menuItem.h"

#ifndef LCDMenu_h
#define LCDMenu_h

enum MENU_ACTION { MENU_ACTION_UP, MENU_ACTION_DOWN, MENU_ACTION_SELECT, MENU_ACTION_BACK, MENU_ACTION_TIME };



class LCDMenu
{
public:
  LCDMenu(uint8_t, uint8_t, uint8_t, Rotary *, OneButton *);
  void setup();
  void printMenu();
  void poll();
  menuItem * firstEntry();
  void addMenuRoot( menuItem * p_menuItem);
  LiquidCrystal_I2C * getLCD();
  void buttonClick();
  void buttonDblClick();
  void DoMenuAction( MENU_ACTION action );
  void MenuUp();
  void MenuDown();
  void MenuSelect();
  void MenuBack();
  void PrintPage( char* pString[], int nLines );
  void getInput( int iMin, int iMax, int iStart, int iSteps, char **label, uint8_t iLabelLines, int * pInt, uint8_t decPlaces = 1 );
  void printInput();
  void intIncrease();
  void intDecrease();
  int getInputInt();
  float getInputFloat();
  void drawInputRow( char *);
  void printLineRight( char*, int );
  void printLine( char*, int );
  void clearLCD();
  void setLiveDisp(char * str, int timeout);
  void pauseMenu();

private:
  LiquidCrystal_I2C *_LCD;
  Rotary *_encoder;
  OneButton *_button;
  int _buttonPressed;
  menuItem* _rootEntry;
  menuItem* _selectedEntry;
  uint8_t _LCDAddress;
  uint8_t _LCDChars;
  uint8_t _LCDRows;
  uint8_t _RotaryA;
  uint8_t _RotaryB;
  uint8_t _buttonPin;
  bool _suspendMenu;
  bool _decInput;
  char _inputBuffer[sizeof(int)*8+1];
  unsigned int _lastMillis;
  bool _liveDisp;
  char * _liveText;
  int _liveTimeout = 0;

  //intInput Vars
  int _intMin;
  int _intMax;
  int _intStep;
  int *_intPtr;
  uint8_t _inDecPrec;

  //strSelect Vars
  
};

#endif
