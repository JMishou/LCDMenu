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
#define EncoderA 5
#define EncoderB 6
#define ButtonPin 7
#define LEDPin 13

#include "LCDMenu.h"

//This example is a Stopwatch and Timer.  Although it is mostly functional, it might not be the best
// user interface.  The layout was created more to provide examples of a stopwatch/timer.


Rotary encoder(EncoderA,EncoderB);
OneButton button(ButtonPin, true);
//Now create the MenuLCD and MenuManager classes.
LCDMenu g_menuLCD(LCDAddr, LCDChars, LCDRows, &encoder, &button);


//Global variables used by the sample application
//when the display is showing user results (e.g. time elapsed), the next "select" should send it back into the menu.
unsigned int g_isDisplaying = false;
int g_timerTime = 23;
char g_timeElapsed[LCDChars];
long g_timerRunning = false;
long g_timerTarget = 0;
long g_autoReset = false;
long g_stopMillis = 0;
long g_startMillis = 0;
unsigned long PrevMillis = 0;


byte g_smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

byte g_frown[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B00000,
  B01110,
  B10001,
};

byte retarrow[8] = {  0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4};


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
 g_menuLCD.setup();

  //Add button handlers
  button.attachClick(buttonClick);
  button.attachDoubleClick(buttonDblClick);

  //Create a menuentry for each parent node
  //Stopwatch will be the first entry and therfor will be the root node.
  menuItem * stopwatch = new menuItem("Stopwatch", NULL, NULL);
  menuItem * timer = new menuItem("Timer", NULL, NULL );
  menuItem * AutoReset = new menuItem("AutoReset", NULL, NULL );

  //Add to stopwatch its siblings (i.e. menu entries on the same level)
  stopwatch->addSibling(timer);
  stopwatch->addSibling(new menuItem( "Credits", NULL, CreditsCallback));
  stopwatch->addSibling(new menuItem( "Draw Smiley", NULL, SmileyCallback));
  stopwatch->addSibling(new menuItem( "Blink LED", NULL,setBlinkCallback));

  //Add to stopwatch each of its child entries
  stopwatch->addChild( new menuItem("Start", NULL, WatchStartCallback) );
  stopwatch->addChild( new menuItem("Stop", NULL, WatchStopCallback ) );
  stopwatch->addChild( new menuItem("Reset", NULL, WatchResetCallback) );
  stopwatch->addChild( new menuItem("Back", (void *) &g_menuLCD, menuItem_BackCallbackFunc));


  //Add to autoreset each of its child entries
  AutoReset->addChild( new menuItem( "Turn Reset On",  (void *) (&g_autoReset), menuItem_BoolTrueCallbackFunc ) );
  AutoReset->addChild( new menuItem( "Turn Reset Off", (void *) (&g_autoReset), menuItem_BoolFalseCallbackFunc ) );
  AutoReset->addChild( new menuItem("Back", (void *) &g_menuLCD, menuItem_BackCallbackFunc) );

  //Add to timer each of its child entries
  timer->addChild( new menuItem("Set Time", NULL, SetTimeCallback ) );
  timer->addChild(AutoReset);
  timer->addChild( new menuItem( "Countdown Start", NULL, TimerStartCallback) );
  timer->addChild( new menuItem( "Countdown Stop", NULL, TimerStopCallback) );
  timer->addChild( new menuItem("Back", (void *) &g_menuLCD, menuItem_BackCallbackFunc) );

  //Set the root entry
  g_menuLCD.addMenuRoot( stopwatch );

  //Create LCD custom chars
  g_menuLCD.getLCD()->createChar( 2, g_smiley );  //Smiley face
  g_menuLCD.getLCD()->createChar( 1, g_frown );   //Frowney face
   g_menuLCD.getLCD()->createChar( 0, retarrow ); //Return Arrow

  //Make sure the menu is drawn correctly after all changes are done
  g_menuLCD.SelectRoot();
  g_menuLCD.printMenu();

}


void setup()
{
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Ready.");
  pinMode(ButtonPin,INPUT_PULLUP);
  pinMode(LEDPin,OUTPUT);
  g_menuLCD.setup();
  setupMenus();
}



void loop()
{
  //Poll the status of the LCDMenu
  g_menuLCD.poll();


  dtostrf( ((float)(millis()-g_startMillis))/1000, 1, 2, g_timeElapsed );
  //Serial.println(g_timeElapsed);

}

//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchStartCallback( char* pMenuText, void *pUserData )
{
  Serial.println("Stopwatch Started");
  g_startMillis = millis();
  char *pTextLines[2] = {"Clock Started", "" };

  dtostrf( (float)0, 1, 2, g_timeElapsed );
  //liveDisplay is an updateable value that updates while the display is currenty up.  In this case it will
  //continually update the elapsed time on the screen after the button is pressed.
  g_menuLCD.printPage(pTextLines, 2);
  g_menuLCD.setLiveDisp(g_timeElapsed, 250 );

  g_menuLCD.pauseMenu();

}


//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchStopCallback( char* pMenuText, void *pUserData )
{
  g_stopMillis = millis();

  char strSeconds[20];
  dtostrf( ((float)(g_stopMillis-g_startMillis))/1000, 1, 2, strSeconds );
  Serial.println(strSeconds);
  char *pTextLines[2] = {"Elapsed Time", strSeconds };
  g_menuLCD.printPage( pTextLines, 2 );  // print page displays text on the screen
  g_menuLCD.pauseMenu();
}

//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void WatchResetCallback( char* pMenuText, void *pUserData )
{
  g_startMillis = 0;
  g_stopMillis = 0;
  char *pTextLines[2] = {"Clock reset", "" };
  g_menuLCD.printPage( pTextLines, 2);
  g_menuLCD.pauseMenu();
}

//This callback uses the built-in Input routine to request input of a integer number from the
//user.  Control will pass to the DoInput function until the user finishes.  the g_timerTime will be set to the
//value the user selects.  This example also uses a callback to trigger an action once the user has selected an input.
void setBlinkCallback( char* pMenuText, void *pUserData )
{
  char *pLabel[3] = {"Set the number of", "blinks for the LED", "to flash"};
  int iNumLabelLines = 3;
  int iMin = 1;
  int iMax = 100;
  int iStart = g_timerTime;
  //Each user input action (such as a turn of rotary enocoder or push of button
  //will step this amount
  int iStep = 1;

  //Do Input will select input from the user.
  g_menuLCD.getInput( iMin, iMax, iStart, iStep, pLabel, iNumLabelLines, &g_timerTime, 0, flashLEDCallback );
}

void flashLEDCallback(int * val){

  for (int x = *val * 2; x > 0; x--){
    digitalWrite(LEDPin,!digitalRead(LEDPin));
    delay(150);
  }
  
}


//This callback uses the built-in Input routine to request input of a integer number from the
//user.  Control will pass to the DoInput function until the user finishes.  the g_timerTime will be set to the
//value the user selects.
void SetTimeCallback( char* pMenuText, void *pUserData )
{
  char *pLabel = "Timer seconds";
  int iNumLabelLines = 1;
  int iMin = 1;
  int iMax = 100;
  int iStart = g_timerTime;
  //Each user input action (such as a turn of rotary enocoder or push of button
  //will step this amount
  int iStep = 1;

  //Do Input will select input from the user.
  g_menuLCD.getInput( iMin, iMax, iStart, iStep, &pLabel, iNumLabelLines, &g_timerTime );
}


//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void TimerStartCallback( char* pMenuText, void *pUserData )
{
  Serial.print( "time = " );
  Serial.println( millis());
  g_timerRunning = true;
  char strSeconds[50];
  itoa( g_timerTime, strSeconds, 10 );
  char *pTextLines[2] = {"Go!", strSeconds };
  g_menuLCD.printPage( pTextLines, 2 );
  g_menuLCD.pauseMenu();

}


//This is a sample callback funtion for when a menu item with no children (aka command) is selected
void TimerStopCallback( char* pMenuText, void *pUserData )
{
  g_timerRunning = false;
  char strSeconds[50];
  itoa( g_timerTime, strSeconds, 10 );
  char *pTextLines[2] = {"Stop!", strSeconds };
  g_menuLCD.printPage( pTextLines, 2 );
  g_menuLCD.pauseMenu();
}

void CreditsCallback( char* pMenuText, void *pUserData )
{
  char *pTextLines[4] = {"Jason Mishou ", "TXRX LABS", "github.com/JMishou","/LCDMenu" };
  g_menuLCD.printPage( pTextLines, 4 );
  delay(5000);
  char *pTextLines2[4] = {"Original code found", "on github @","github.com","/DavidAndrews"};
  g_menuLCD.printPage( pTextLines2, 4 );
    delay(5000);
    g_menuLCD.printMenu();
}


void SmileyCallback( char* pMenuText, void *pUserData )
{
  for( int i = 0; i < 10 ; ++i )
  {
    g_menuLCD.clearLCD();
    g_menuLCD.getLCD()->setCursor( 8,0 );
    g_menuLCD.getLCD()->print( (char)2 );
    delay(500);
    g_menuLCD.clearLCD();
    g_menuLCD.getLCD()->setCursor( 8,0 );
    g_menuLCD.getLCD()->print( (char)1 );
    delay(500);
  }
  g_menuLCD.printMenu();
}


//Here we set the button input to execute the menu function button click
void buttonClick(){
  g_menuLCD.buttonClick();
}

//Here we set the button input to execute the menu function button double click
void buttonDblClick(){
  g_menuLCD.buttonDblClick();
}
