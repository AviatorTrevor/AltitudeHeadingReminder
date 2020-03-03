/*
Author: Trevor Bartlett
Email: aviatortrevor@gmail.com
Cell: (760) 832-3480

Hardware device to remind pilots of assigned headings & altitudes. Also has buzzer feature
to alert the pilot of when he/she is approaching altitude, or departed from it.

*TODO:
*alternating buzzer 4000 & 4100 frequencies for 200ft alert
*add settings to disable 200ft and 1000ft alarms
*after 20 seconds of inactivity, reset the page and cursor to the main screen
*test sleeping
*test recovering pressure sensor?
*add more data fields to settings page
*interrupts causing an interrupt to beeping noises
*     https://forum.arduino.cc/index.php?topic=175511.0
*     http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
*buy louder buzzer? or add 2nd buzzer? Try different frequencies using
*
*
*LOUD FREQUENCIES TO USE?
*1400
*2000
*2100
*2300
*2400
*4100 & 4200 alternating?
*/
#include <EEPROM.h>
#include <SFE_BMP180.h> //TODO implement your own BMP180 pressure sensor library so that we can have a slim version to cut down on program storage space
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //TODO implement your own graphics libraries so that we can have a slim version to cut down on program storage space
#include <util/atomic.h> //TODO: remove?
#include <avr/sleep.h>

#define DEBUG

#define cOneSecond                     1000 //1000 milliseconds = 1 second
#define cOneSecondBeforeOverflow       (unsigned long)(pow(2, sizeof(unsigned long) * 8) - cOneSecond)
#define cTwentySeconds                 20 * cOneSecond
#define cFeetInMeters                  3.28084
#define cSeaLevelPressureMb            1013.25 //standard sea level pressure in millibars
#define cSeaLevelPressureInHg          29.92
#define cFtLabel                       "ft"
#define cInHgLabel                     "\"Hg"
#define cAltitudeSelectIncrement       100   //ft
#define cAltitudeFineSelectIncrement   10    //ft
#define cInitialSelectedAltitudeOffset 2000  //ft
#define cAltitudeHighSelectIncrement   1000  //ft
#define cHighAltitude                  18000 //ft
#define cLowestAltitudeSelect          -1500 //ft
#define cHighestAltitudeSelect         60000 //ft
#define cHighestAltitudeAlert          20000 //ft, the pressure sensor will only measure so high. No point in alerting above a certain pressure level
#define cAltimeterSettingMin           27.50 //inHg
#define cAltimeterSettingMax           31.50 //inHg
#define cAltimeterSettingInterval      0.01  //inHg
#define cCalibrationOffsetMin          -990  //ft
#define cCalibrationOffsetMax          990   //ft
#define cCalibrationOffsetInterval     10    //ft
#define cHeadingSelectIncrement        5     //degrees
#define cTrueAltitudeRoundToNearest    10    //ft
#define cTryToRecover                  false

//EEPROM
#define         cEepromWriteDelay              5000  //milliseconds
volatile bool   gNeedToWriteToEeprom;

//Main program variables
double          gTrueAltitudeDouble;           //feet, true altitude
volatile long   gSelectedAltitudeLong; //feet
volatile double gAltimeterSettingDouble = cSeaLevelPressureInHg; //inches of mercury
volatile int    gCalibratedAltitudeOffsetInt;
volatile int    gSelectedHeadingInt = 360; //degrees

//Buzzer
#define            cInitialSelectedAltitudeTimeout 5000 //determine initial selection altitude within 5 seconds, or give up and allow it to stay at 0 because pressure sensor failed
#define            cAlarm200ToGo                   200
#define            cAlarm1000ToGo                  1000
#define            cBuzzPin                        6
#define            cBuzzFrequencyA                 4100 //Hz frequency for the Musical Note B-7 is 3951 (which is what Garmin uses??). 4000Hz seems to resonate better with this speaker
#define            cBuzzFrequencyB                 4200
#define            cLongBuzzDuration               1000
#define            cShortBuzzOnFreqADuration       80
#define            cShortBuzzOnFreqBDuration       80
#define            cUrgentBuzzNumberOfBeeps        8
#define            cDisableAlarmKnobMovementTime   1200
enum BuzzAlarmMode {Climbing1000ToGo, Climbing200ToGo, Descending1000ToGo, Descending200ToGo, AltitudeDeviate, BeepingAlarm, AlarmDisabled, DetermineAlarmState};
BuzzAlarmMode      gAlarmModeEnum = AlarmDisabled;
BuzzAlarmMode      gNextAlarmModeEnum; //only used to get out of a multiple-beep alarm state
int                gBuzzCountInt; //always a value between [0, cUrgentBuzzNumberOfBeeps]

//Timing control
unsigned long          gNextSensorBeginCycleTs;
unsigned long          gNextSensorReadyTs;
unsigned long          gNextScreenRefreshTs; //TODO is this being used?
unsigned long          gNextBuzzTs;
volatile unsigned long gLeftButtonPressedTs;
volatile unsigned long gLastAltitudeSelectChangeTs;
volatile unsigned long gLeftRotaryReleaseTs;
volatile unsigned long gEepromSaveNeededTs;
volatile unsigned long gLastRotaryActionTs;

//Display
#define cFlashCursorOnPeriod     800
#define cFlashCursorOffPeriod    200
#define cSplashScreenDelay       cOneSecond
#define cMaxScreenRefreshRate    30 //30Hz //TODO
#define cMaxScreenRefreshPeriod  (cOneSecond / cMaxScreenRefreshRate)
#define cDisplayAddr             0x3C
#define cScreenWidth             128 // OLED display width, in pixels
#define cScreenHeight            32  // OLED display height, in pixels
#define cOledReset               4   // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 gDisplay(cScreenWidth, cScreenHeight, &Wire, cOledReset);

//Page & Cursor control
enum DisplayPage {DisplayPageMain, DisplayPageSettings} gDisplayPage = DisplayPageMain;
enum Cursor {CursorSelectHeading, CursorSelectAltimeter, CursorSelectOffset} gCursor = CursorSelectHeading;

//BMP180 Sensor variables
#define    cSensorLoopCycle               2 //2Hz
#define    cSensorLoopPeriod              (cOneSecond / cSensorLoopCycle)
#define    cBmp180Quality                 3 //highest quality, more electrical draw [0-3]. Wait times are {5ms, 8ms, 14ms, 26ms}. Getting temperature is always 5ms.
SFE_BMP180 gSensor;
char       gSensorStatusChar;             //byte value BMP180 pressure sensor returns. Acts as success status as well as time to wait for next state
int        gSensorProcessStateInt;        //0 = start measuring temperature, 1 = get temperature, 2 = start measuring pressure, 3 = get pressure
double     gSensorTemperatureDouble;      //celcius
double     gSensorPressureDouble;         //millibars

//Rotary Knobs
#define cRotaryStates              4
#define cDtLookup                  0 //row 0 in cEncoderValues matrix lookup
#define cClkLookup                 1 //row 1 in cEncoderValues matrix lookup
#define cLongButtonPress           1500
#define cRotaryButtonReleaseDelay  60
#define PRESSED                    LOW
#define RELEASED                   HIGH


//Rotary encoder signal pairs, DT CLK - clockwise order
//When DT & CLK are both 1, the rotary is sitting on a detent
const int cEncoderValues[2][cRotaryStates] = {
  1, 1, 0, 0, //DT
  1, 0, 0, 1  //CLK
};

//Right Rotary Knob
#define        cPinRightRotarySignalDt  8
#define        cPinRightRotarySignalClk 9
#define        cPinRightRotaryButton    10
volatile int   gRightRotaryIndex;
volatile int   gRightRotaryButton = RELEASED;
volatile int   gRightRotaryDirection;

//Left Rotary Knob
#define        cPinLeftRotarySignalDt  2
#define        cPinLeftRotarySignalClk 3
#define        cPinLeftRotaryButton    4
volatile int   gLeftRotaryIndex;
volatile int   gLeftRotaryButton = RELEASED;
volatile int   gLeftRotaryButtonPreviousValue = RELEASED;
volatile int   gLeftRotaryDirection;
volatile bool  gLeftButtonPossibleLongPress;
volatile bool  gLeftRotaryFineTuningPress;
volatile bool  gDisableLeftRotaryProcessing; //we disable knob processing right after the screen changes pages until the button is released

//Error Codes
#define cBMP180InitFail       10
#define cBMP180TempStartFail  20
#define cBMP180TempGetFail    30
#define cBMP180PressStartFail 40
#define cBMP180PressGetFail   50
#define cDisplayInitFail      60

//Error handling variables
bool eBMP180Failed;
bool eDisplayError;
int  eErrorCode;
bool eUserAcknowledgedError;

//////////////////////////////////////////////////////////////////////////
void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  println("APPLICATION START");

  initializeValuesFromEeprom();
  initializeBmp180Sensor();
  initializeDisplayDevice();
  initializeRotaryKnobs();
  initializeBuzzer();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

//////////////////////////////////////////////////////////////////////////
void initializeValuesFromEeprom() {
  int eepromIndex = 0;
  double tempDouble;
  int tempInt;
  
  //Last Altimeter Setting
  tempDouble = gAltimeterSettingDouble; //this silences a compiler warning
  EEPROM.get(eepromIndex, tempDouble);
  gAltimeterSettingDouble = tempDouble;
  if (gAltimeterSettingDouble < cAltimeterSettingMin || gAltimeterSettingDouble > cAltimeterSettingMax)
    gAltimeterSettingDouble = cSeaLevelPressureInHg;
  eepromIndex += sizeof(gAltimeterSettingDouble);
  
  
  //Last altitude offset
  tempInt = gCalibratedAltitudeOffsetInt; //this silences a compiler warning
  EEPROM.get(eepromIndex, tempInt);
  gCalibratedAltitudeOffsetInt = tempInt;
  if (gCalibratedAltitudeOffsetInt < cCalibrationOffsetMin || gCalibratedAltitudeOffsetInt > cCalibrationOffsetMax)
    gCalibratedAltitudeOffsetInt = 0;
  
  
  //units - feet, meters
  //pressure units - inHg, millibars??? Pascals???
  //pressure sensor type
  //rotary encoder type
  //display type
  //1000ft alert tone
  //200ft alert tone
  //departing altitude by 200ft alert tone
  //device version [MajorVersion.HardwareBuildVersion.SoftwareVersion]
}

//////////////////////////////////////////////////////////////////////////
void initializeBmp180Sensor() {
  delay(300);
  if (eBMP180Failed = !gSensor.begin()) {
    eErrorCode = cBMP180InitFail;
  }
}

//////////////////////////////////////////////////////////////////////////
void initializeDisplayDevice() {
  if(eDisplayError = !gDisplay.begin(SSD1306_SWITCHCAPVCC, cDisplayAddr)) {
    eErrorCode = cDisplayInitFail;
  }
  else {
    //Show splash screen
    gDisplay.clearDisplay();
    gDisplay.setTextSize(2);
    //TODO set font
    gDisplay.setTextColor(SSD1306_WHITE);
    gDisplay.setCursor(18,11);
    gDisplay.println(F("POWER ON"));
    gDisplay.display();
    delay(cSplashScreenDelay);
  }
}

//////////////////////////////////////////////////////////////////////////
void initializeRotaryKnobs() {
  //TODO: I still don't understand how pullups change anything or why it's suggested to use them
  pinMode(cPinRightRotarySignalDt, INPUT_PULLUP);
  pinMode(cPinRightRotarySignalClk, INPUT_PULLUP);
  pinMode(cPinRightRotaryButton, INPUT_PULLUP);

  gLeftRotaryButtonPreviousValue = gLeftRotaryButton = digitalRead(cPinLeftRotaryButton);
  gRightRotaryButton = digitalRead(cPinRightRotaryButton);
  gDisableLeftRotaryProcessing = (gLeftRotaryButton == PRESSED); //Disable left knob processing if we started with the knob-button being pressed
  

  int rightRotaryDt  = digitalRead(cPinRightRotarySignalDt);
  int rightRotaryClk = digitalRead(cPinRightRotarySignalClk);
  int leftRotaryDt   = digitalRead(cPinLeftRotarySignalDt);
  int leftRotaryClk  = digitalRead(cPinLeftRotarySignalClk);

  //Find what phase the right rotary knobs are in
  for (int i = 0; i < cRotaryStates; i++) {
    if (rightRotaryDt == cEncoderValues[cDtLookup][i] && rightRotaryClk == cEncoderValues[cClkLookup][i]) {
      gRightRotaryIndex = i;
    }
    if (leftRotaryDt == cEncoderValues[cDtLookup][i] && leftRotaryClk == cEncoderValues[cClkLookup][i]) {
      gLeftRotaryIndex = i;
    }
  }


  //setup interrupts for left-rotary knob
  *digitalPinToPCMSK(cPinLeftRotarySignalDt)
      |= bit (digitalPinToPCMSKbit(cPinLeftRotarySignalDt))
      |  bit (digitalPinToPCMSKbit(cPinLeftRotarySignalClk))
      |  bit (digitalPinToPCMSKbit(cPinLeftRotaryButton));

  PCIFR 
      |= bit (digitalPinToPCICRbit(cPinLeftRotarySignalDt))
      |  bit (digitalPinToPCICRbit(cPinLeftRotarySignalClk))
      |  bit (digitalPinToPCICRbit(cPinLeftRotaryButton));
  PCICR
      |= bit (digitalPinToPCICRbit(cPinLeftRotarySignalDt))
      |  bit (digitalPinToPCICRbit(cPinLeftRotarySignalClk))
      |  bit (digitalPinToPCICRbit(cPinLeftRotaryButton));

  //setup interrupts for right-rotary knob
  *digitalPinToPCMSK(cPinRightRotarySignalDt)
      |= bit (digitalPinToPCMSKbit(cPinRightRotarySignalDt))
      |  bit (digitalPinToPCMSKbit(cPinRightRotarySignalClk))
      |  bit (digitalPinToPCMSKbit(cPinRightRotaryButton));

  PCIFR 
      |= bit (digitalPinToPCICRbit(cPinRightRotarySignalDt))
      |  bit (digitalPinToPCICRbit(cPinRightRotarySignalClk))
      |  bit (digitalPinToPCICRbit(cPinRightRotaryButton));
  PCICR
      |= bit (digitalPinToPCICRbit(cPinRightRotarySignalDt))
      |  bit (digitalPinToPCICRbit(cPinRightRotarySignalClk))
      |  bit (digitalPinToPCICRbit(cPinRightRotaryButton));
}

//////////////////////////////////////////////////////////////////////////
void initializeBuzzer() {
  pinMode(cBuzzPin, OUTPUT);
}












//////////////////////////////////////////////////////////////////////////
//Main Loop
//////////////////////////////////////////////////////////////////////////
void loop() {
  if (millis() > cOneSecondBeforeOverflow) { //this handles the extremely rare case (every ~50 days of uptime) that the clock overflows
    gNextSensorBeginCycleTs = gNextSensorReadyTs = gNextScreenRefreshTs = gNextBuzzTs = 0; //reset timing
    delay(cOneSecond); //we take a 1 second frozen penalty for handling this extremely rare situation
    return; //return so that we grab a new currentTime
  }
  
  if (millis() >= gNextSensorReadyTs) {
    handleBmp180Sensor();
  }

  if (gLeftButtonPossibleLongPress && millis() - gLeftButtonPressedTs >= cLongButtonPress) {
    handleLeftRotaryLongPress();
    gLeftButtonPossibleLongPress = false;
  }

  if (gDisplayPage != DisplayPageMain && millis() - gLastRotaryActionTs >= cTwentySeconds) {
    gDisplayPage = DisplayPageMain;
    gCursor = CursorSelectHeading;
  }

  handleErrors();
  handleBuzzer();
  handleDisplay();

  if (gNeedToWriteToEeprom && millis() - gEepromSaveNeededTs >= cEepromWriteDelay) {
    writeValuesToEeprom();
  }
  //TODO calculate VSI (vertical speed) just to display for fun?
}

//////////////////////////////////////////////////////////////////////////
void handleBmp180Sensor() {
  if (!eBMP180Failed) {
    switch (gSensorProcessStateInt) {
      
      
      case 0: //Start measuring temperature
      gNextSensorBeginCycleTs = millis() + cSensorLoopPeriod;
      gSensorStatusChar = gSensor.startTemperature();
      gNextSensorReadyTs = millis() + gSensorStatusChar; //sensor tells us when it's ready for the next step
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
        eErrorCode = cBMP180TempStartFail;
      }
      break;
      
      
      case 1: //Retrieve temperature measured
      gSensorStatusChar = gSensor.getTemperature(gSensorTemperatureDouble); //TODO: I've never tested out the accuracy of the temp sensor
      gNextSensorReadyTs = millis(); //ready for the next step immediately
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
        eErrorCode = cBMP180TempGetFail;
      }
      break;
      
      
      case 2: //Start measuring pressure
      gSensorStatusChar = gSensor.startPressure(cBmp180Quality);
      gNextSensorReadyTs = millis() + gSensorStatusChar;//sensor tells us when it's ready for the next step
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
        eErrorCode = cBMP180PressStartFail;
      }
      break;
      
      
      case 3: //Retrieve pressure measured
      gNextSensorReadyTs = gNextSensorBeginCycleTs; //we'll start the process over again according to the sensor rate we defined in the constants section
      gSensorStatusChar = gSensor.getPressure(gSensorPressureDouble, gSensorTemperatureDouble);
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
        eErrorCode = cBMP180PressGetFail;
      }
      else { //only update the true altitude if the pressure reading was valid
        gTrueAltitudeDouble = altitudeCorrected(cFeetInMeters * gSensor.altitude(gSensorPressureDouble, cSeaLevelPressureMb));
      };
    }
    if (eBMP180Failed) {
      gSensorProcessStateInt = 0;
    }
    else {
      gSensorProcessStateInt = (gSensorProcessStateInt + 1) % 4;
    }
  }
  else if (cTryToRecover) {
    initializeBmp180Sensor();
    gNextSensorReadyTs = millis() + 5000; //TODO look at this later
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotary() {
  //The button being pressed can lead to 1 of 3 outcomes: {Short Press, Long Press, a rotation occuring before the long press time is reached}
  gLeftRotaryButton = digitalRead(cPinLeftRotaryButton); //read button state
  int leftRotaryDt = digitalRead(cPinLeftRotarySignalDt);
  int leftRotaryClk = digitalRead(cPinLeftRotarySignalClk);

  gLastRotaryActionTs = millis();

  if (gLeftRotaryButton != gLeftRotaryButtonPreviousValue && millis() - gLeftRotaryReleaseTs >= cRotaryButtonReleaseDelay) { //if button state changed
    gLeftRotaryButtonPreviousValue = gLeftRotaryButton;
    if (gLeftRotaryButton == PRESSED) {
      gLeftButtonPressedTs = millis();
      gLeftButtonPossibleLongPress = true;
      gLeftRotaryFineTuningPress = false;
    }
    else if (millis() - gLeftButtonPressedTs < cLongButtonPress && !gLeftRotaryFineTuningPress) { //released after short press that wasn't a fine-tuning event
      gLeftButtonPossibleLongPress = false;
      gLeftRotaryReleaseTs = millis();
      handleLeftRotaryShortPress();
    }
    else { //(gLeftRotaryButton == RELEASED) //released after either a long-press or a fine-tuning press
      gLeftButtonPossibleLongPress = false;
      gDisableLeftRotaryProcessing = false;
      gLeftRotaryReleaseTs = millis();
    }
  }
  
  //Handle rotation
  int clockwiseRotationIndex = (gLeftRotaryIndex + 1) % cRotaryStates;
  int counterclockwiseRotationIndexIndex = (gLeftRotaryIndex - 1 + cRotaryStates) % cRotaryStates;
  if (leftRotaryDt == cEncoderValues[cDtLookup][clockwiseRotationIndex] && leftRotaryClk == cEncoderValues[cClkLookup][clockwiseRotationIndex]) {
    gLeftRotaryDirection++;
    gLeftRotaryIndex = clockwiseRotationIndex;
  }
  else if (leftRotaryDt == cEncoderValues[cDtLookup][counterclockwiseRotationIndexIndex]
        && leftRotaryClk == cEncoderValues[cClkLookup][counterclockwiseRotationIndexIndex]) {
    gLeftRotaryDirection--;
    gLeftRotaryIndex = counterclockwiseRotationIndexIndex;
  }

  if (gLeftRotaryIndex == 0) { //if we reached a detent on the rotary (gLeftRotaryIndex == 0)
    //either we moved clockwise, counter-clockwise, or wiggled a little back without hitting a neighboring detent
    handleLeftRotaryMovement(gLeftRotaryDirection / cRotaryStates);
    gLeftRotaryDirection = 0;
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotaryMovement(int increment) {
  if (increment == 0 || gDisableLeftRotaryProcessing) {
    return; //if we didn't really move detents, do nothing
  }
  gLeftButtonPossibleLongPress = false;
  gLeftRotaryFineTuningPress = (gLeftRotaryButton == PRESSED); //not all states have a fine-tuning press, but we still have to set this here because we don't want a short-press to register and move cursor

  switch (gCursor) {
    case CursorSelectHeading:
    {
      if (gLeftRotaryFineTuningPress) {
        gSelectedHeadingInt = (gSelectedHeadingInt + increment + 359) % 360 + 1;
      }
      else {
        int incrementMagnitude = cHeadingSelectIncrement;
        if (gSelectedHeadingInt % cHeadingSelectIncrement != 0) {
          incrementMagnitude = cHeadingSelectIncrement / 2; //we are at an in-between cHeadingSelectIncrement state, so the knob movement will increment or decement to the nearest cHeadingSelectIncrement
        }
        gSelectedHeadingInt = roundNumber((gSelectedHeadingInt + increment * incrementMagnitude + 359) % 360 + 1, cHeadingSelectIncrement);
      }
      break;
    }
    
    case CursorSelectAltimeter:
    {
      gAltimeterSettingDouble = constrain(gAltimeterSettingDouble + cAltimeterSettingInterval * increment, cAltimeterSettingMin, cAltimeterSettingMax);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
    }
    
    case CursorSelectOffset:
    {
      gCalibratedAltitudeOffsetInt = constrain(gCalibratedAltitudeOffsetInt + cCalibrationOffsetInterval * increment, cCalibrationOffsetMin, cCalibrationOffsetMax);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
    }
    
    default:
      println("Unexpected default case for handling rotary knob rotation");
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotaryShortPress() {
  if (gDisplayPage == DisplayPageMain) {
    if (gCursor == CursorSelectHeading) {
      gCursor = CursorSelectAltimeter;
    }
    else {
      gCursor = CursorSelectHeading;
    }
  }
  else if (gDisplayPage == DisplayPageSettings) {
    //TODO
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotaryLongPress() {
  gDisableLeftRotaryProcessing = true;
  gLeftRotaryFineTuningPress = false;
  if (gDisplayPage == DisplayPageMain) { //if you're on the main page, go to the settings page
    gDisplayPage = DisplayPageSettings;
    gCursor = CursorSelectOffset; //put your cursor on the offset selection value
  }
  else if (gDisplayPage == DisplayPageSettings) { //if you're on the settings page, go the main page
    gDisplayPage = DisplayPageMain;
    gCursor = CursorSelectHeading; //set your cursor to the heading you select
  }
}

//////////////////////////////////////////////////////////////////////////
void handleRightRotary() {
  //The button being pressed on the right knob can only be used for fine-tuning mode. A released state indicates normal altitude selection mode.
  gRightRotaryButton = digitalRead(cPinRightRotaryButton); //read button state
  int rightRotaryDt = digitalRead(cPinRightRotarySignalDt);
  int rightRotaryClk = digitalRead(cPinRightRotarySignalClk);

  gLastRotaryActionTs = millis();

  int clockwiseRotationIndex = (gRightRotaryIndex + 1) % cRotaryStates;
  int counterclockwiseRotationIndexIndex = (gRightRotaryIndex - 1 + cRotaryStates) % cRotaryStates;
  if (rightRotaryDt == cEncoderValues[cDtLookup][clockwiseRotationIndex] && rightRotaryClk == cEncoderValues[cClkLookup][clockwiseRotationIndex]) {
    gRightRotaryDirection++;
    gRightRotaryIndex = clockwiseRotationIndex;
  }
  else if (rightRotaryDt == cEncoderValues[cDtLookup][counterclockwiseRotationIndexIndex]
        && rightRotaryClk == cEncoderValues[cClkLookup][counterclockwiseRotationIndexIndex]) {
    gRightRotaryDirection--;
    gRightRotaryIndex = counterclockwiseRotationIndexIndex;
  }

  if (gRightRotaryIndex == 0) { //if we reached a detent on the rotary (gRightRotaryIndex == 0)
    //either we moved clockwise, counter-clockwise, or wiggled a little back without hitting a neighboring detent
    handleRightRotaryMovement(gRightRotaryDirection / cRotaryStates);
    gRightRotaryDirection = 0;
  }
}

//////////////////////////////////////////////////////////////////////////
void handleRightRotaryMovement(int increment) { // +1 indicates rotation to the right, -1 indicates rotation to the left
  if (increment == 0) {
    return; //if we didn't really move detents, do nothing
  }
  if (gDisplayPage != DisplayPageMain) { //if you rotated the altitude select knob, go back to the main page
    gDisplayPage = DisplayPageMain;
    gCursor = CursorSelectHeading; //set your cursor to the heading you select
    return;
  }
  gLastAltitudeSelectChangeTs = millis(); //note the time the knob moved to a different detent so we silence the alarm/buzzer

  gAlarmModeEnum = AlarmDisabled; //disable alarm if we change selected altitude
  if (gSelectedAltitudeLong > cHighAltitude || (gSelectedAltitudeLong == cHighAltitude && increment == 1) ) {
    int incrementMagnitude = cAltitudeHighSelectIncrement; //normal increment magnitude indicates the button being released and the current selected altitude being on an interval
    int rounding = cAltitudeHighSelectIncrement;
    if (gRightRotaryButton == PRESSED) { //if we're fine-tuning, make the increment magnitude smaller
      incrementMagnitude = cAltitudeSelectIncrement;
      rounding = cAltitudeSelectIncrement;
    }
    else if (gSelectedAltitudeLong % cAltitudeHighSelectIncrement != 0) { //if we're using the bigger increment but are in-between intervals, then we are going to jump to the nearest interval based on the direction of turn
      incrementMagnitude = cAltitudeHighSelectIncrement / 2; //we are at an in-between cAltitudeHighSelectIncrement state, so the knob movement will increment or decement to the nearest cAltitudeHighSelectIncrement
    }
    gSelectedAltitudeLong = min(roundNumber(gSelectedAltitudeLong + increment * incrementMagnitude, rounding), cHighestAltitudeSelect);
  }
  else {
    int incrementMagnitude = cAltitudeSelectIncrement;
    int rounding = cAltitudeSelectIncrement;
    if (gRightRotaryButton == PRESSED) {
      incrementMagnitude = cAltitudeFineSelectIncrement;
      rounding = cAltitudeFineSelectIncrement;
    }
    else if (gSelectedAltitudeLong % cAltitudeSelectIncrement != 0) {
      incrementMagnitude = cAltitudeSelectIncrement / 2; //we are at an in-between cAltitudeSelectIncrement state, so the knob movement will increment or decement to the nearest cAltitudeSelectIncrement
    }
    gSelectedAltitudeLong = max(roundNumber(gSelectedAltitudeLong + increment * incrementMagnitude, rounding), cLowestAltitudeSelect);
  }
}

//////////////////////////////////////////////////////////////////////////
void handleBuzzer() {
  switch (gAlarmModeEnum) {
    case Climbing1000ToGo:
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm1000ToGo) {
        tone(cBuzzPin, cBuzzFrequencyA, cLongBuzzDuration);
        gAlarmModeEnum = Climbing200ToGo;
      }
      break;
    
    case Climbing200ToGo:
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = BeepingAlarm;
        gNextAlarmModeEnum = AltitudeDeviate;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      else if (gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm1000ToGo) {
        gAlarmModeEnum = Climbing1000ToGo;
      }
      break;

    case Descending1000ToGo:
      if (gTrueAltitudeDouble <= gSelectedAltitudeLong + cAlarm1000ToGo) {
        tone(cBuzzPin, cBuzzFrequencyA, cLongBuzzDuration);
        gAlarmModeEnum = Descending200ToGo;
      }
      break;
      
    case Descending200ToGo:
      if (gTrueAltitudeDouble <= gSelectedAltitudeLong + cAlarm200ToGo) {
        gAlarmModeEnum = BeepingAlarm;
        gNextAlarmModeEnum = AltitudeDeviate;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      else if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm1000ToGo) {
        gAlarmModeEnum = Descending1000ToGo;
      }
      break;
    
    case AltitudeDeviate: //We're looking to sound the alarm if pilot deviates from his altitude he already reached
      if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm200ToGo || gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = BeepingAlarm; //initiate beeping the alarm on the next pass
        gNextBuzzTs = millis(); //next buzz time is now
        if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm200ToGo) {
          gNextAlarmModeEnum = Descending200ToGo;
        }
        else if (gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm200ToGo) {
          gNextAlarmModeEnum = Climbing200ToGo;
        }
      }
      break;
      
    case BeepingAlarm:
      if (gBuzzCountInt == 0) {
        gBuzzCountInt = cUrgentBuzzNumberOfBeeps;
      }
      if (millis() >= gNextBuzzTs) {
        if (gBuzzCountInt % 2 == 0) { //every other cycle, change frequency
          tone(cBuzzPin, cBuzzFrequencyA);
          gNextBuzzTs = millis() + cShortBuzzOnFreqADuration;
        }
        else {
          tone(cBuzzPin, cBuzzFrequencyB);
          gNextBuzzTs = millis() + cShortBuzzOnFreqBDuration;
        }
        
        gBuzzCountInt--;
        if (gBuzzCountInt == 0) {
          gAlarmModeEnum = gNextAlarmModeEnum;
          noTone(cBuzzPin);
        }
      }
      break;

    case AlarmDisabled: //Alarm can be disabled either because we just started, or the rotary knob has moved recently
      //The very first time we start up and get a pressure reading, we initialize what the altitude selection is to a little higher than current altitude
      noTone(cBuzzPin); //stop the buzzer
      if (eBMP180Failed || eDisplayError)
        break;
      else if (gSelectedAltitudeLong == 0 && millis() < cInitialSelectedAltitudeTimeout) { //this statement is only for initializing the first altitude selection value above the current altitude
        long initialAltitudeSelection = gTrueAltitudeDouble + cInitialSelectedAltitudeOffset;
        if (initialAltitudeSelection >= cHighAltitude) {
          gSelectedAltitudeLong = roundNumber(initialAltitudeSelection + cAltitudeHighSelectIncrement, cAltitudeHighSelectIncrement);
        }
        else {
          gSelectedAltitudeLong = roundNumber(initialAltitudeSelection + cAltitudeSelectIncrement * 5, cAltitudeSelectIncrement * 5);
        }
      }
      else if (gSelectedAltitudeLong <= cHighestAltitudeAlert && millis() - gLastAltitudeSelectChangeTs >= cDisableAlarmKnobMovementTime) {
        gAlarmModeEnum = DetermineAlarmState;
      }
      break;

    default: //default case
    case DetermineAlarmState:
      long diffBetweenSelectionAndTrueAltitude = gSelectedAltitudeLong - gTrueAltitudeDouble;
      if (gSelectedAltitudeLong > cHighestAltitudeAlert || eBMP180Failed || eDisplayError) {
        gAlarmModeEnum = AlarmDisabled;
      }
      else if (diffBetweenSelectionAndTrueAltitude > cAlarm1000ToGo) {
        gAlarmModeEnum = Climbing1000ToGo;
      }
      else if (diffBetweenSelectionAndTrueAltitude < (cAlarm1000ToGo * -1)) {
        gAlarmModeEnum = Descending1000ToGo;
      }
      else if (diffBetweenSelectionAndTrueAltitude > cAlarm200ToGo) {
        gAlarmModeEnum = Climbing200ToGo;
      }
      else if (diffBetweenSelectionAndTrueAltitude < (cAlarm200ToGo * -1)){ 
        gAlarmModeEnum = Descending200ToGo;
      }
      else /*(diffBetweenSelectionAndTrueAltitude > (cAlarm200ToGo * -1) && diffBetweenSelectionAndTrueAltitude < cAlarm200ToGo)*/ { //I commented out the conditional check to save the computation cycles, because it's the only remaining logical choice
        gAlarmModeEnum = AltitudeDeviate;
      }
      break;
  }
}

//////////////////////////////////////////////////////////////////////////
void handleDisplay() {
  //If we're having display issues, try to recover
  if (eDisplayError && cTryToRecover) {
    initializeDisplayDevice();
    if (eDisplayError) //if we're still having trouble, return
      return;
  }
  
  if (gDisplayPage == DisplayPageMain) { //normal display more, display the altitude
    handleDisplayOfMainPage();
  }
  else if (gDisplayPage == DisplayPageSettings) { //settings display
    handleDisplayOfSettingsPage();
  }
}

//////////////////////////////////////////////////////////////////////////
void handleDisplayOfMainPage() {
  gDisplay.invertDisplay(false); //TODO remove?
  gDisplay.clearDisplay();
  gDisplay.setTextSize(1);

  //show the altimeter top-left...
  gDisplay.setCursor(0, 0);
  if (gCursor == CursorSelectAltimeter) {
    gDisplay.println(">" + String(gAltimeterSettingDouble) + "\"");
  }
  else {
    gDisplay.println(" " + String(gAltimeterSettingDouble) + "\"");
  }

  //show the sensor true altitude
  if (eBMP180Failed) { //if there's a sensor error, the top line should be the error message
      gDisplay.setCursor(62, 0);
    gDisplay.println("SENSOR FAIL"); //TODO: in the future, write the error code to EEPROM
  }
  else if (gSelectedAltitudeLong > cHighestAltitudeAlert || gTrueAltitudeDouble > cHighestAltitudeAlert + cAlarm200ToGo) {
    gDisplay.setCursor(68, 0);
    gDisplay.println("SENSOR OFF");
  }
  else { //...show the current altitude top-right
    gDisplay.setCursor(80, 0);
    gDisplay.println(displayNumber(roundNumber(gTrueAltitudeDouble, cTrueAltitudeRoundToNearest)) + cFtLabel);
  }

  //...Display Selected Altitude
  gDisplay.setTextSize(2);
  gDisplay.setCursor(32, 15);
  long temporarySelectedAltitude = gSelectedAltitudeLong;
  gDisplay.println(displayNumber(temporarySelectedAltitude) + cFtLabel);

  //Display Selected Heading
  gDisplay.setTextSize(1);
  gDisplay.setCursor(0, 15);
  int tempSelectedHeading = gSelectedHeadingInt;
  char heading[6];
  sprintf(heading, "%c%03d%c", gCursor == CursorSelectHeading ? '>' : ' ', tempSelectedHeading, (char)(247));
  gDisplay.println(String(heading));

  gDisplay.display();
}

//////////////////////////////////////////////////////////////////////////
void handleDisplayOfSettingsPage() {
  gDisplay.invertDisplay(false); //TODO remove?
  gDisplay.clearDisplay();
  gDisplay.setTextSize(1);
  gDisplay.setCursor(0, 0);
  
  if (gCursor == CursorSelectOffset) {
    if (gCalibratedAltitudeOffsetInt > 0) {
      gDisplay.println("> Offset +" + String(gCalibratedAltitudeOffsetInt) + cFtLabel);
    }
    else {
      gDisplay.println("> Offset " + String(gCalibratedAltitudeOffsetInt) + cFtLabel);
    }
  }
  else {
     if (gCalibratedAltitudeOffsetInt > 0) {
      gDisplay.println("  Offset +" + String(gCalibratedAltitudeOffsetInt) + cFtLabel);
    }
    else {
      gDisplay.println("  Offset " + String(gCalibratedAltitudeOffsetInt) + cFtLabel);
    }
  }
  gDisplay.display();
}























//////////////////////////////////////////////////////////////////////////
ISR (PCINT0_vect) {    // handle pin change interrupt for D8 to D13 here
  handleRightRotary();
}

//////////////////////////////////////////////////////////////////////////
ISR (PCINT2_vect) {    // handle pin change interrupt for D0 to D7 here
  handleLeftRotary();
}

//////////////////////////////////////////////////////////////////////////
// we only write data to EEPROM if the value changes, because EEPROM has
// a limited number of writes, but unlimited reads
//////////////////////////////////////////////////////////////////////////
void writeValuesToEeprom() {
  gNeedToWriteToEeprom = false;
  int eepromIndex = 0;
  
  //Last Altimeter Setting
  double altimeterSetting;
  EEPROM.get(eepromIndex, altimeterSetting);
  if (altimeterSetting != gAltimeterSettingDouble) {
    altimeterSetting = gAltimeterSettingDouble; //this silences a compiler warning
    EEPROM.put(eepromIndex, altimeterSetting);
  }
  eepromIndex += sizeof(gAltimeterSettingDouble);

  //Last altitude offset
  int altitudeOffset;
  EEPROM.get(eepromIndex, altitudeOffset);
  if (altitudeOffset != gCalibratedAltitudeOffsetInt) {
    altitudeOffset = gCalibratedAltitudeOffsetInt; //this silences a compiler warning
    EEPROM.put(eepromIndex, altitudeOffset);
  }
  //TODO eepromIndex += sizeof(gCalibratedAltitudeOffsetInt);
}

//////////////////////////////////////////////////////////////////////////
double altitudeCorrected(double pressureAltitude) {
  return (pressureAltitude - (1 - (pow(gAltimeterSettingDouble / cSeaLevelPressureInHg, 0.190284))) * 145366.45) + gCalibratedAltitudeOffsetInt;
}

//////////////////////////////////////////////////////////////////////////
// Prints number with commas, but only supports [0, 999999]
// Because the char[] is a fixed length, this essentially creates the
// illusion of right-aligned text on the display
//////////////////////////////////////////////////////////////////////////
String displayNumber(const long &number) {
  int thousands = static_cast<int>(number / 1000);
  int ones = static_cast<int>(number % 1000);
  char result[8];

  if (number >= 1000) {
    sprintf(result, "%01d,%03d", thousands, ones);
  }
  else if (number <= -1000) {
    sprintf(result, "%01d,%03d", thousands, abs(ones));
  }
  else {
    sprintf(result, "%01d", ones);
  }
  char temp[8];
  strncpy(temp, result, 7);
  sprintf(result, "% 6s", temp); //pad with leading spaces
  return String(result);
}

//////////////////////////////////////////////////////////////////////////
long roundNumber(const long &number, const int &roundNearest) {
  int sign = (number < 0) ? -1 : 1; //positive or negative number
  return (number + sign * roundNearest / 2) / roundNearest * roundNearest;
}

//////////////////////////////////////////////////////////////////////////
long roundNumber(const double &number, const int &roundNearest) {
  int sign = (number < 0) ? -1 : 1; //positive or negative number
  return static_cast<long>((number + sign * roundNearest / 2) / roundNearest) * roundNearest;
}

//////////////////////////////////////////////////////////////////////////
int roundNumber(const int &number, const int &roundNearest) {
  int sign = (number < 0) ? -1 : 1; //positive or negative number
  return (number + sign * roundNearest / 2) / roundNearest * roundNearest;
}

//////////////////////////////////////////////////////////////////////////
void handleErrors() {
  /*if (eBMP180Failed) {
    println(String("SENSOR FAILED CODE ") + String(eErrorCode)); //TODO print out error codes
  }
  if (eDisplayError) {
    println(String("DISPLAY FAILED"));
  }*/
}

//////////////////////////////////////////////////////////////////////////
void println(String msg) {
  #ifdef DEBUG
  Serial.println(msg);
  #endif
}
