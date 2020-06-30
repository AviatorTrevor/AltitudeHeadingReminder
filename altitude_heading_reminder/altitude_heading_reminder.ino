/*
Author: Trevor Bartlett
Email: aviatortrevor@gmail.com

Hardware device to remind pilots of assigned headings & altitudes. Also has buzzer feature
to alert the pilot of when he/she is approaching altitude, or departed from it.

*TODO:
*implement F macro for string to save program memory space
*implement dual OLED displays
  *multiplexer or alternative method
  *rotating display option
  *flash/invert screen when alerting
  *dim mode?
  *custom library to cut size
*handle EEPROM writes
  *byte 0 stores the next available EEPROM value. It can't reassign unless the value can fit before reaching 1024 - piracy values
  *bytes [1-X] stores index of EEPROM items. needs to be set to zero when piracy code is entered
  *each EEPROM value must also store a value for number of writes to EEPROM so we can know when to abandon that chunk of EEPROM
*option to show text upside down for different mounting options?
*add settings to disable 200ft and 1000ft alarms
*test sleeping
*interrupts causing an interrupt to beeping noises
*design for battery
*software for battery level & charging
*create software license - credit for libraries used
*     https://forum.arduino.cc/index.php?topic=175511.0
*     http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
*     https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
*buy louder buzzer? Try different frequencies using
*
*
*LOUD FREQUENCIES TO USE?
*1400
*2000
*2100
*2300
*2400
*4100 & 4200 alternating?long
*/
#include <EEPROM.h>
#include <SFE_BMP180.h> //TODO implement your own BMP180 pressure sensor library so that we can have a slim version to cut down on program storage space
#include <Wire.h>
#include <Adafruit_GFX.h> //TODO temporary for now
#include <Adafruit_SSD1306.h> //TODO temporary for now

#define DEBUG //TODO remove

#define cAppVersion                    "1.0.0" //[HardwareConfig].[MajorSoftwareRelease].[MinorSoftwareRelease]
#define cAppCodeNumberOfDigits         6
#define cAppCodeOne                    8
#define cAppCodeTwo                    8
#define cAppCodeThree                  1
#define cAppCodeFour                   6
#define cAppCodeFive                   6
#define cAppCodeSix                    6
#define cOneSecond                     1000 //1000 milliseconds = 1 second
#define cOneSecondBeforeOverflow       (unsigned long)(pow(2, sizeof(unsigned long) * 8) - cOneSecond)
#define cTenSeconds                    10000
#define cMillisecondsInMinute          60000
#define cFeetInMeters                  3.28084
#define cSeaLevelPressureHPa           1013.25 //standard sea level pressure in millibars
#define cSeaLevelPressureInHg          29.92 //standard sea level pressure in inches of mercury
#define cFtLabel                       "ft"
#define cFpmLabel                      "fpm" //feet per minute
#define cInLabel                       "\""
#define cHPaLabel                      "hPa"
#define cMetersLabel                   "m"
#define cDegFLabel                     'F'
#define cAltitudeSelectIncrement       100   //ft
#define cAltitudeFineSelectIncrement   10    //ft
#define cInitialSelectedAltitudeOffset 2000  //ft
#define cAltitudeHighSelectIncrement   1000  //ft
#define cHighAltitude                  18000 //ft
#define cLowestAltitudeSelect          -1500 //ft
#define cHighestAltitudeSelect         60000 //ft
#define cHighestAltitudeAlert          24000 //ft, the pressure sensor will only measure so high. No point in alerting above a certain pressure level
#define cAltimeterSettingInHgMin       27.50 //inHg
#define cAltimeterSettingInHgMax       31.50 //inHg
#define cAltimeterSettingInHgInterval  0.01  //inHg
#define cCalibrationOffsetMin          -990  //ft
#define cCalibrationOffsetMax          990   //ft
#define cCalibrationOffsetInterval     10    //ft
#define cHeadingSelectIncrement        5     //degrees
#define cTrueAltitudeRoundToNearestFt  10    //ft
#define cTrueAltitudeRoundToNearestM   10    //meters

//EEPROM
#define         cSizeOfEeprom          1024
#define         cEepromWriteDelay      10000  //milliseconds
#define         cEepromMaxWrites       90000
volatile bool   gNeedToWriteToEeprom;

//BMP180 Sensor variables
#define    cSensorLoopCycle               2 //2Hz
#define    cSensorLoopPeriod              (cOneSecond / cSensorLoopCycle)
#define    cBmp180Quality                 3 //highest quality, more electrical draw [0-3]. Wait times are {5ms, 8ms, 14ms, 26ms}. Getting temperature is always 5ms.
SFE_BMP180 gSensor;
bool       gInitializedAltitude;
char       gSensorStatusChar;             //byte value BMP180 pressure sensor returns. Acts as success status as well as time to wait for next state
int        gSensorProcessStateInt;        //0 = start measuring temperature, 1 = get temperature, 2 = start measuring pressure, 3 = get pressure
double     gSensorTemperatureDouble;      //celcius
double     gSensorPressureDouble;         //millibars
enum SensorMode {SensorModeOnShow, SensorModeOnHide, SensorModeOff, cNumberOfSensorModes};
volatile SensorMode gSensorMode = SensorModeOnShow;

//Main program variables
double          gTrueAltitudeDouble;
volatile long   gSelectedAltitudeLong;
volatile double gAltimeterSettingInHgDouble = cSeaLevelPressureInHg;
volatile int    gCalibratedAltitudeOffsetInt;
volatile int    gPermanentCalibratedAltitudeOffsetInt;
volatile int    gSelectedHeadingInt = 360; //degrees
volatile int    gSelectAppCode = 0;
volatile int    gAppCodeSequence = 0;
bool            gLegitimate = true;
enum AltitudeUnits {AltitudeUnitsFeet, AltitudeUnitsMeters, cNumberOfAltitudeUnits};
enum PressureUnits {PressureUnitsInHg, PressureUnitsHPa, cNumberOfPressureUnits};
volatile AltitudeUnits gAltitudeUnits = AltitudeUnitsFeet;
volatile PressureUnits gPressureUnits = PressureUnitsInHg;

//Buzzer
#define            cAlarm200ToGo                   200  //ft
#define            cAlarm1000ToGo                  1000 //ft
#define            cBuzzPin                        6
#define            cBuzzFrequencyA                 4100 //Hz frequency for the Musical Note B-7 is 3951 (which is what Garmin uses??). 4000 & 4100Hz seems to resonate better with this speaker
#define            cBuzzFrequencyB                 2400 //Hz
#define            cLongBuzzDuration               1000
#define            cShortBuzzOnFreqADuration       150
#define            cShortBuzzOnFreqBDuration       150
#define            cUrgentBuzzNumberOfBeeps        7
#define            cDisableAlarmKnobMovementTime   1200
#define            cDisableAlarmAfterAlarmTime     1500
#define            cDisableBacklightTimePeriod     200
#define            cDisableBacklightPriorToAlarm   300
enum BuzzAlarmMode {Climbing1000ToGo, Climbing200ToGo, Descending1000ToGo, Descending200ToGo, AltitudeDeviate, UrgentAlarm, AlarmDisabled, DetermineAlarmState};
BuzzAlarmMode      gAlarmModeEnum = AlarmDisabled;
int                gBuzzCountInt; //always a value between [0, cUrgentBuzzNumberOfBeeps]

//Timing control
unsigned long          gNextSensorBeginCycleTs;
unsigned long          gNextSensorReadyTs;
unsigned long          gNextBuzzTs;
unsigned long          gLastAlarmTs;
unsigned long          gLastBacklightOffTs;
unsigned long          gTimerStartTs;
volatile unsigned long gLeftButtonPressedTs;
volatile unsigned long gRightButtonPressedTs;
volatile unsigned long gLastRightRotaryActionTs;
volatile unsigned long gLeftRotaryReleaseTs;
volatile unsigned long gRightRotaryReleaseTs;
volatile unsigned long gEepromSaveNeededTs;
volatile unsigned long gLastRotaryActionTs;

//Display
#define cOledAddr      0x3C
#define cOledWidth     128
#define cOledHeight    32
#define cOledReset     4
Adafruit_SSD1306 gOled(cOledWidth, cOledHeight, &Wire, cOledReset);
#define cMaxScreenRefreshRate     30 //30Hz //TODO remove?
#define cMaxScreenRefreshPeriod   (cOneSecond / cMaxScreenRefreshRate) //TODO remove?
volatile bool gOledDim = false;
volatile bool gDeviceFlipped = false;
volatile bool gShowLeftScreen = false; //TODO temporary, remove

char gDisplayTopLeftContent[8];
char gDisplayBottomLeftContent[8];
char gDisplayTopRightContent[9];
char gDisplayBottomRightContent[7];

//Cursor control
enum Cursor {
    CursorSelectHeading,
    CursorSelectAltimeter,
    CursorSelectTimer,
    CursorSelectBrightness,
    CursorSelectOffset,
    CursorSelectSensor,
    CursorSelectAltitudeUnits,
    CursorSelectPressureUnits,
    CursorViewBmpTemp,
    CursorViewBatteryLevel,
    cNumberOfCursorModes }
    gCursor = CursorSelectHeading;
    //TODO: set volume?
    //TODO: set tones?
    //TODO: turn alerts on/off?
    //TODO: current temperature?

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
volatile int   gRightRotaryButtonPreviousValue = RELEASED;
volatile int   gRightRotaryDirection;
volatile bool  gRightButtonPossibleLongPress;
volatile bool  gRightRotaryFineTuningPress;
volatile bool  gDisableRightRotaryProcessing; //we disable knob processing right after the altitude-sync command until the button is released

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

//Error handling variables
bool eBMP180Failed;

//////////////////////////////////////////////////////////////////////////
void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  println("APPLICATION START"); //TODO remove?
  #endif

  initializeDisplayDevice();
  initializeValuesFromEeprom();
  initializeBmp180Sensor();
  initializeRotaryKnobs();
  initializeBuzzer();
  initializePiracyCheck();
  delay(1200); //delay for splash screen (to see version number)
}

//////////////////////////////////////////////////////////////////////////
void initializeValuesFromEeprom() {
  int eepromIndex = 0;
  double tempDouble;
  int tempInt;
  bool tempBool;
  
  //Last Altimeter Setting
  EEPROM.get(eepromIndex, tempDouble);
  if (isnan(tempDouble) || tempDouble < cAltimeterSettingInHgMin || tempDouble > cAltimeterSettingInHgMax) {
    gAltimeterSettingInHgDouble = cSeaLevelPressureInHg;
  }
  else {
    gAltimeterSettingInHgDouble = tempDouble;
  }
  eepromIndex += sizeof(double);
  
  
  //Last altitude offset
  EEPROM.get(eepromIndex, tempInt);
  gCalibratedAltitudeOffsetInt = tempInt;
  if (gCalibratedAltitudeOffsetInt < cCalibrationOffsetMin || gCalibratedAltitudeOffsetInt > cCalibrationOffsetMax) {
    gCalibratedAltitudeOffsetInt = 0;
  }
  eepromIndex += sizeof(int);


  //Semi-permanent altitude offset
  EEPROM.get(eepromIndex, tempInt);
  gPermanentCalibratedAltitudeOffsetInt = tempInt;
  eepromIndex += sizeof(int);


  //Last Screen Brightness
  EEPROM.get(eepromIndex, tempBool);
  gOledDim = tempBool;
  eepromIndex += sizeof(bool);


  //Sensor Mode - On/Show, On/Hide, Off
  EEPROM.get(eepromIndex, tempInt);
  gSensorMode = static_cast<SensorMode>(constrain(tempInt, 0, cNumberOfSensorModes));
  eepromIndex += sizeof(int);


  //Altitude Units
  EEPROM.get(eepromIndex, tempInt);
  gAltitudeUnits = static_cast<AltitudeUnits>(constrain(tempInt, 0, cNumberOfAltitudeUnits));
  eepromIndex += sizeof(int);


  //Pressure Units
  EEPROM.get(eepromIndex, tempInt);
  gPressureUnits = static_cast<PressureUnits>(constrain(tempInt, 0, cNumberOfPressureUnits));
  eepromIndex += sizeof(int);


  //TODO
  //1000ft alert tone
  //200ft alert tone
  //departing altitude by 200ft alert tone
}

//////////////////////////////////////////////////////////////////////////
void initializePiracyCheck() {
  int eepromIndex = cSizeOfEeprom - sizeof(int) * cAppCodeNumberOfDigits;
  int codeValue;

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeOne) {
    gLegitimate = false;
  }
  eepromIndex += sizeof(int);

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeTwo) {
    gLegitimate = false;
  }
  eepromIndex += sizeof(int);

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeThree) {
    gLegitimate = false;
  }
  eepromIndex += sizeof(int);

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeFour) {
    gLegitimate = false;
  }
  eepromIndex += sizeof(int);

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeFive) {
    gLegitimate = false;
  }
  eepromIndex += sizeof(int);

  EEPROM.get(eepromIndex, codeValue);
  if (codeValue != cAppCodeSix) {
    gLegitimate = false;
  }

  if (gLegitimate) {
    return;
  }
  gOled.clearDisplay();
  gOled.setCursor(13,0);
  gOled.setTextSize(2);
  gOled.print("ERROR 69");
  delay(3000);
  gOled.clearDisplay();
  int lastWrittenSequence = 0;
  int currentAppCodeSequence = gAppCodeSequence;
  int selectAppCode = gSelectAppCode;
  eepromIndex = cSizeOfEeprom - sizeof(int) * cAppCodeNumberOfDigits;
  while (!gLegitimate && currentAppCodeSequence <= cAppCodeNumberOfDigits) {
    selectAppCode = gSelectAppCode;
    gOled.setCursor(0,0);
    sprintf(gDisplayTopLine, "%016d", selectAppCode);
    gOled.print(gDisplayTopLine);
    if (currentAppCodeSequence > lastWrittenSequence) {
      EEPROM.update(eepromIndex, selectAppCode);
      lastWrittenSequence = currentAppCodeSequence;
      eepromIndex += sizeof(int);
      gSelectAppCode = 0;
      if (currentAppCodeSequence == cAppCodeNumberOfDigits) {
        while (true) {
          gOled.clearDisplay();
          gOled.setCursor(13,0);
          gOled.print("RESTART");
          delay(999999999);
        }
      }
    }
    currentAppCodeSequence = gAppCodeSequence;
  }
}

//////////////////////////////////////////////////////////////////////////
void initializeBmp180Sensor() {
  eBMP180Failed = !gSensor.begin();
}

//////////////////////////////////////////////////////////////////////////
void initializeDisplayDevice() {
  gOled.begin(SSD1306_SWITCHCAPVCC, cOledAddr);
  gOled.invertDisplay(false);
  gOled.clearDisplay();
  gOled.setTextSize(2);
  gOled.setCursor(12,0);
  gOled.print("Version");
  gOled.setCursor(15, 17);
  gOled.print(cAppVersion);
  gOled.display();
}

//////////////////////////////////////////////////////////////////////////
void initializeRotaryKnobs() {
  //TODO: I still don't understand how pullups change anything or why it's suggested to use them
  pinMode(cPinRightRotarySignalDt, INPUT_PULLUP);
  pinMode(cPinRightRotarySignalClk, INPUT_PULLUP);
  pinMode(cPinRightRotaryButton, INPUT_PULLUP);

  gLeftRotaryButtonPreviousValue = gLeftRotaryButton = digitalRead(cPinLeftRotaryButton);
  gRightRotaryButtonPreviousValue =gRightRotaryButton = digitalRead(cPinRightRotaryButton);
  gDisableLeftRotaryProcessing = (gLeftRotaryButton == PRESSED); //Disable left knob processing if we started with the knob-button being pressed
  gDisableRightRotaryProcessing = (gRightRotaryButton == PRESSED); //Disable right knob processing if we started with the knob-button being pressed
  

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
    gNextSensorBeginCycleTs = gNextSensorReadyTs = gNextBuzzTs = 0; //reset timing
    delay(cOneSecond); //we take a 1 second frozen penalty for handling this extremely rare situation
    return; //return so that we grab a new currentTime
  }

  if (gSelectedAltitudeLong <= cHighestAltitudeAlert && millis() >= gNextSensorReadyTs) {
    handleBmp180Sensor();
  }

  //check & handle long-press of left rotary knob
  if (gLeftButtonPossibleLongPress && millis() - gLeftButtonPressedTs >= cLongButtonPress) {
    handleLeftRotaryLongPress();
  }

  //check & handle long-press of right rotary knob
  if (gRightButtonPossibleLongPress && millis() - gRightButtonPressedTs >= cLongButtonPress) {
    handleRightRotaryLongPress();
  }

  if (gCursor != CursorSelectHeading && gCursor != CursorSelectAltimeter && gCursor != CursorSelectTimer && millis() - gLastRotaryActionTs >= cTenSeconds) {
    gCursor = CursorSelectHeading;
  }

  if (gSensorMode != SensorModeOff) {
    handleBuzzer();
  }
  handleDisplay();

  if (gNeedToWriteToEeprom && millis() - gEepromSaveNeededTs >= cEepromWriteDelay) {
    writeValuesToEeprom();
  }
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
      }
      break;
      
      
      case 1: //Retrieve temperature measured
      gSensorStatusChar = gSensor.getTemperature(gSensorTemperatureDouble);
      gNextSensorReadyTs = millis(); //ready for the next step immediately
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
      }
      break;
      
      
      case 2: //Start measuring pressure
      gSensorStatusChar = gSensor.startPressure(cBmp180Quality);
      gNextSensorReadyTs = millis() + gSensorStatusChar;//sensor tells us when it's ready for the next step
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
      }
      break;
      
      
      case 3: //Retrieve pressure measured
      gNextSensorReadyTs = gNextSensorBeginCycleTs; //we'll start the process over again according to the sensor rate we defined in the constants section
      gSensorStatusChar = gSensor.getPressure(gSensorPressureDouble, gSensorTemperatureDouble);
      if (gSensorStatusChar == 0) {
        eBMP180Failed = true;
      }
      else { //only update the true altitude if the pressure reading was valid
        gTrueAltitudeDouble = altitudeCorrected(cFeetInMeters * gSensor.altitude(gSensorPressureDouble, cSeaLevelPressureHPa));
        //if this is the first pressure reading, initialize selected altitude
        if (!gInitializedAltitude) {
          gInitializedAltitude = true; //this if statement only gets called once
          long initialAltitudeSelection = gTrueAltitudeDouble + cInitialSelectedAltitudeOffset;
          if (initialAltitudeSelection >= cHighAltitude) {
            gSelectedAltitudeLong = roundNumber(initialAltitudeSelection + cAltitudeHighSelectIncrement, cAltitudeHighSelectIncrement);
          }
          else {
            gSelectedAltitudeLong = roundNumber(initialAltitudeSelection + cAltitudeSelectIncrement * 5, cAltitudeSelectIncrement * 5);
          }
        }
      }
    }
    if (eBMP180Failed) {
      gSensorProcessStateInt = 0;
    }
    else {
      gSensorProcessStateInt = (gSensorProcessStateInt + 1) % 4;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotary() {
  gShowLeftScreen = true;
  //The button being pressed can lead to 1 of 3 outcomes: {Short Press, Long Press, a rotation occuring before the long press time is reached}
  gLeftRotaryButton = digitalRead(cPinLeftRotaryButton); //read button state
  int leftRotaryDt = digitalRead(cPinLeftRotarySignalDt);
  int leftRotaryClk = digitalRead(cPinLeftRotarySignalClk);

  if (!gLegitimate) {
    if (gLeftRotaryButton == PRESSED && gSelectAppCode != 0) {
      gAppCodeSequence++;
    }
  }
  else {
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

  if (!gLegitimate) {
    gSelectAppCode += increment;
    return;
  }
  
  gLeftButtonPossibleLongPress = false;
  gLeftRotaryFineTuningPress = (gLeftRotaryButton == PRESSED); //not all states have a fine-tuning press, but we still have to set this here because we don't want a short-press to register and move cursor
  int incrementMagnitude;

  switch (gCursor) {
    case CursorSelectHeading:
      if (gLeftRotaryFineTuningPress) {
        gSelectedHeadingInt = (gSelectedHeadingInt + increment + 359) % 360 + 1;
      }
      else {
        incrementMagnitude = cHeadingSelectIncrement;
        if (gSelectedHeadingInt % cHeadingSelectIncrement != 0) {
          incrementMagnitude = cHeadingSelectIncrement / 2; //we are at an in-between cHeadingSelectIncrement state, so the knob movement will increment or decement to the nearest cHeadingSelectIncrement
        }
        gSelectedHeadingInt = roundNumber((gSelectedHeadingInt + increment * incrementMagnitude + 359) % 360 + 1, cHeadingSelectIncrement);
      }
      break;
    
    case CursorSelectAltimeter:
      gAltimeterSettingInHgDouble = constrain(gAltimeterSettingInHgDouble + cAltimeterSettingInHgInterval * increment, cAltimeterSettingInHgMin, cAltimeterSettingInHgMax);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;

    case CursorSelectTimer:
      if (increment > 0 && gTimerStartTs == 0) {
        gTimerStartTs = millis() - 750;
      }
      else if (increment < 0) {
        gTimerStartTs = 0;
      }
      break;

    case CursorSelectBrightness:
      gOledDim = !gOledDim;
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
    
    case CursorSelectOffset:
      gCalibratedAltitudeOffsetInt = constrain(roundNumber(gCalibratedAltitudeOffsetInt + cCalibrationOffsetInterval * increment, cCalibrationOffsetInterval), cCalibrationOffsetMin, cCalibrationOffsetMax);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
      
    case CursorSelectSensor:
      gSensorMode = static_cast<SensorMode>((gSensorMode + increment + cNumberOfSensorModes) % cNumberOfSensorModes);
      if (gSensorMode == SensorModeOff) {
        gAlarmModeEnum = AlarmDisabled;
      }
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
      
    case CursorSelectAltitudeUnits:
      gAltitudeUnits = static_cast<AltitudeUnits>((gAltitudeUnits + increment + cNumberOfAltitudeUnits) % cNumberOfAltitudeUnits);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;

    case CursorSelectPressureUnits:
      gPressureUnits = static_cast<PressureUnits>((gPressureUnits + increment + cNumberOfPressureUnits) % cNumberOfPressureUnits);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;
      
    case CursorViewBatteryLevel:
      break; //do nothing for these modes, display only
    
    default:
      println("Unexpected default case for handling left rotary knob rotation");
  }
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotaryShortPress() {
  gCursor = static_cast<Cursor>((gCursor + 1 + cNumberOfCursorModes) % cNumberOfCursorModes);
}

//////////////////////////////////////////////////////////////////////////
void handleLeftRotaryLongPress() {
  gLeftButtonPossibleLongPress = false;
  gDisableLeftRotaryProcessing = true;
  gLeftRotaryFineTuningPress = false;
  gCursor = CursorSelectHeading;

  if (gSelectedHeadingInt == 333 && gSelectedAltitudeLong == -1500) { //magic numbers to program offset
    gPermanentCalibratedAltitudeOffsetInt += gCalibratedAltitudeOffsetInt;
    gCalibratedAltitudeOffsetInt = 0;
    gEepromSaveNeededTs = millis();
    gNeedToWriteToEeprom = true;
  }
}

//////////////////////////////////////////////////////////////////////////
void handleRightRotary() {
  gShowLeftScreen = false;
  //The button being pressed on the right knob can only be used for fine-tuning mode or altitude-sync (long press). A released state indicates normal altitude selection mode.
  gRightRotaryButton = digitalRead(cPinRightRotaryButton); //read button state
  int rightRotaryDt = digitalRead(cPinRightRotarySignalDt);
  int rightRotaryClk = digitalRead(cPinRightRotarySignalClk);

  gLastRotaryActionTs = millis();

  if (gRightRotaryButton != gRightRotaryButtonPreviousValue && millis() - gRightRotaryReleaseTs >= cRotaryButtonReleaseDelay) { //if button state changed
    gRightRotaryButtonPreviousValue = gRightRotaryButton;
    if (gRightRotaryButton == PRESSED) {
      gRightButtonPressedTs = millis();
      gRightButtonPossibleLongPress = true;
      gRightRotaryFineTuningPress = false;
    }
    else { //(gRightRotaryButton == RELEASED) //released after either a long-press or a fine-tuning press
      gRightButtonPossibleLongPress = false;
      gDisableRightRotaryProcessing = false;
      gRightRotaryReleaseTs = millis();
    }
  }

  //Handle rotation
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
  gLastRightRotaryActionTs = millis(); //note the time the knob moved to a different detent so we silence the alarm/buzzer
  gRightButtonPossibleLongPress = false;
  gRightRotaryFineTuningPress = (gRightRotaryButton == PRESSED);

  gAlarmModeEnum = AlarmDisabled; //disable alarm if we change selected altitude
  if (gSelectedAltitudeLong > cHighAltitude || (gSelectedAltitudeLong == cHighAltitude && increment == 1) ) {
    int incrementMagnitude = cAltitudeHighSelectIncrement; //normal increment magnitude indicates the button being released and the current selected altitude being on an interval
    int rounding = cAltitudeHighSelectIncrement;
    if (gRightRotaryFineTuningPress) { //if we're fine-tuning, make the increment magnitude smaller
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
    if (gRightRotaryFineTuningPress) {
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
void handleRightRotaryLongPress() {
  gRightButtonPossibleLongPress = false;
  gDisableRightRotaryProcessing = true;
  gRightRotaryFineTuningPress = false;
  gLastRightRotaryActionTs = millis(); //note the time so we silence the alarm/buzzer temporarily
  gAlarmModeEnum = AlarmDisabled; //disable alarm

  //if altitude is above 18k (cHighAltitude), then set selected altitude to nearest 1000ft (cAltitudeHighSelectIncrement) of our true altitude
  if (gTrueAltitudeDouble >= cHighAltitude) {
    gSelectedAltitudeLong = roundNumber(gTrueAltitudeDouble, cAltitudeHighSelectIncrement);
  }
  else { //else, we're below 18k, so round to the nearest 100ft (cAltitudeSelectIncrement) of our true altitude
    gSelectedAltitudeLong = roundNumber(gTrueAltitudeDouble, cAltitudeSelectIncrement);
  }
}

//////////////////////////////////////////////////////////////////////////
void handleBuzzer() {
  switch (gAlarmModeEnum) {
    case Climbing1000ToGo:
    {
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm1000ToGo) {
        tone(cBuzzPin, cBuzzFrequencyA, cLongBuzzDuration);
        gLastAlarmTs = millis() + cLongBuzzDuration;
        gAlarmModeEnum = AlarmDisabled;
      }
      break;
    }

    case Descending1000ToGo:
    {
      if (gTrueAltitudeDouble <= gSelectedAltitudeLong + cAlarm1000ToGo) {
        tone(cBuzzPin, cBuzzFrequencyA, cLongBuzzDuration);
        gLastAlarmTs = millis() + cLongBuzzDuration;
        gAlarmModeEnum = AlarmDisabled;
      }
      break;
    }
    
    case Climbing200ToGo:
    {
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = UrgentAlarm;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      else if (gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm1000ToGo) {
        gAlarmModeEnum = Climbing1000ToGo;
      }
      break;
    }
      
    case Descending200ToGo:
    {
      if (gTrueAltitudeDouble <= gSelectedAltitudeLong + cAlarm200ToGo) {
        gAlarmModeEnum = UrgentAlarm;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      else if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm1000ToGo) {
        gAlarmModeEnum = Descending1000ToGo;
      }
      break;
    }
    
    case AltitudeDeviate: //We're looking to sound the alarm if pilot deviates from his altitude he already reached
    {
      if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm200ToGo || gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = UrgentAlarm; //initiate beeping the alarm on the next pass
        gNextBuzzTs = millis(); //next buzz time is now
      }
      break;
    }
      
    case UrgentAlarm:
    {
      if (gBuzzCountInt == 0) {
        gLastAlarmTs = millis();
        gBuzzCountInt = cUrgentBuzzNumberOfBeeps + 1;
      }
      if (millis() >= gNextBuzzTs) {
        gBuzzCountInt--;
        if (gBuzzCountInt != 0 && gBuzzCountInt % 2 == 0) { //every other cycle, change frequency
          tone(cBuzzPin, cBuzzFrequencyB);
          gNextBuzzTs = millis() + cShortBuzzOnFreqBDuration;
        }
        else if (gBuzzCountInt % 2 == 1) {
          tone(cBuzzPin, cBuzzFrequencyA);
          gNextBuzzTs = millis() + cShortBuzzOnFreqADuration;
        }
        else {
          gAlarmModeEnum = AlarmDisabled;
          noTone(cBuzzPin);
        }
      }
      break;
    }

    case AlarmDisabled:
    {
      if (millis() - gLastAlarmTs < cDisableAlarmAfterAlarmTime) {
        //do nothing
        break;
      }

      if (eBMP180Failed || !gInitializedAltitude) {
        if (millis() > 5000) {
          gInitializedAltitude = true;
        }
      }
      else if (gSensorMode != SensorModeOff && gSelectedAltitudeLong <= cHighestAltitudeAlert && millis() - gLastRightRotaryActionTs >= cDisableAlarmKnobMovementTime && millis() - gLastAlarmTs >= cDisableAlarmAfterAlarmTime) {
        gAlarmModeEnum = DetermineAlarmState;
      }
      else {
        noTone(cBuzzPin); //stop the buzzer
      }
      break;
    }

    default: //default case
    case DetermineAlarmState:
    {
      long diffBetweenSelectionAndTrueAltitude = gSelectedAltitudeLong - gTrueAltitudeDouble;
      if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || eBMP180Failed) {
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
}

//////////////////////////////////////////////////////////////////////////
void handleDisplay() {
  if (gShowLeftScreen) {
    drawLeftScreen();
  }
  else {
    drawRightScreen();
  }
}

//////////////////////////////////////////////////////////////////////////
void drawLeftScreen() {
  switch (gCursor) {
    case CursorSelectHeading: //Display Selected Heading
    {
      sprintf(gDisplayTopLeftContent, "%s", "Hdg");
      sprintf(gDisplayBottomLeftContent, "%03d%c", gSelectedHeadingInt, (char)(223)); //223 == degree symbol
      break;
    }

    case CursorSelectAltimeter:
      sprintf(gDisplayTopLeftContent, "%-s", "Altmtr");
      if (gPressureUnits == PressureUnitsInHg) {
        sprintf(gDisplayBottomLeftContent, "%d.%02d" cInLabel, (int)gAltimeterSettingInHgDouble, (int)(gAltimeterSettingInHgDouble*100)%100);
      }
      else { //(gPressureUnits == PressureUnitsHPa) {
        sprintf(gDisplayBottomLeftContent, "%s" cHPaLabel, String(static_cast<int>(gAltimeterSettingInHgDouble * cSeaLevelPressureHPa/cSeaLevelPressureInHg)).c_str()); //TODO fix this to use sprintf like a few lines above
      }
      break;

    case CursorSelectTimer:
      sprintf(gDisplayTopLeftContent, "%-s", "Timer");
      if (gTimerStartTs == 0) {
        sprintf(gDisplayBottomLeftContent, "00:00");
      }
      else {
        unsigned long seconds = (millis() - gTimerStartTs) / 1000;
        unsigned long minutes = seconds / 60;
        while (minutes >= 100) {
          minutes -= 100;
        }
        seconds -= minutes * 60;
        sprintf(gDisplayBottomLeftContent, "%02d:%02d", (int)minutes, (int)seconds);
      }
      break;

    case CursorSelectBrightness:
      sprintf(gDisplayTopLeftContent, "%s", "Dim");
      sprintf(gDisplayBottomLeftContent, "%-7d", gOledDim ? "ON" : "OFF");
      break;

    case CursorSelectOffset:
      sprintf(gDisplayTopLeftContent, "%-s", "Offset");
      sprintf(gDisplayBottomLeftContent, "%+d" cFtLabel, gCalibratedAltitudeOffsetInt);
      break;

    case CursorSelectSensor:
      sprintf(gDisplayTopLeftContent, "%s", "Sensor");
      if (gSensorMode == SensorModeOnShow) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "ON/SHOW");
      }
      else if (gSensorMode == SensorModeOnHide) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "ON/HIDE");
      }
      else {
        sprintf(gDisplayBottomLeftContent, "%-7s", "OFF");
      }
      break;

    case CursorSelectAltitudeUnits:
      sprintf(gDisplayTopLeftContent, "%-7s", "A Units");
      if (gAltitudeUnits == AltitudeUnitsFeet) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "Ft");
      }
      else { //(gAltitudeUnits == AltitudeUnitsMeters) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "Meters");
      }
      break;

    case CursorSelectPressureUnits:
      sprintf(gDisplayTopLeftContent, "%-7s", "P Units");
      if (gPressureUnits == PressureUnitsInHg) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "\"Hg");
      }
      else { //(gPressureUnits == PressureUnitsHPa) {
        sprintf(gDisplayBottomLeftContent, "%-7s", "hPa");
      }
      break;

    case CursorViewBmpTemp:
    {
      sprintf(gDisplayTopLeftContent, "%-7s", "Temp");
      double temperatureFarenheit = gSensorTemperatureDouble * 9 / 5 + 32;
      sprintf(gDisplayBottomLeftContent, "%d.%d%c%c", (int)temperatureFarenheit, (int)(temperatureFarenheit*10)%10, (char)(223), cDegFLabel);
      break;
    }

    case CursorViewBatteryLevel:
      sprintf(gDisplayTopLeftContent, "%-7s", "Batt");
      sprintf(gDisplayBottomLeftContent, "%-7s", "100%"); //TODO implement battery level
      break;
  }

  if (gOledDim) {
    gOled.ssd1306_command(SSD1306_SETCONTRAST); //0x81
    gOled.ssd1306_command(0);
    
    gOled.ssd1306_command(SSD1306_SETPRECHARGE); //0xD9
    gOled.ssd1306_command(0);

    gOled.ssd1306_command(SSD1306_SETVCOMDETECT); //0xDB
    gOled.ssd1306_command(0);
  }
  else {
    gOled.ssd1306_command(SSD1306_SETCONTRAST); //0x81
    gOled.ssd1306_command(255);
    
    gOled.ssd1306_command(SSD1306_SETPRECHARGE); //0xD9
    gOled.ssd1306_command(255);

    gOled.ssd1306_command(SSD1306_SETVCOMDETECT); //0xDB
    gOled.ssd1306_command(255);
  }
  gOled.clearDisplay();
  gOled.setRotation(2);
  gOled.setTextColor(SSD1306_WHITE);
  gOled.setTextSize(1);
  gOled.setCursor(0, 0);
  gOled.println(gDisplayTopLeftContent);
  gOled.setTextSize(3);
  gOled.setCursor(0, 11);
  gOled.println(gDisplayBottomLeftContent);
  gOled.setTextSize(2);
  gOled.setCursor(25, 11);
  gOled.print(cFtLabel);
  gOled.display();
}

//////////////////////////////////////////////////////////////////////////
void drawRightScreen() {
  //show the sensor true altitude
  if (eBMP180Failed) { //if there's a sensor error, the top line should be the error message
    sprintf(gDisplayTopRightContent, "%8s", "FAIL");
  }
  else if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || gTrueAltitudeDouble > cHighestAltitudeAlert + cAlarm200ToGo) {
    sprintf(gDisplayTopRightContent, "%8s", "OFF");
  }
  else if (gSensorMode == SensorModeOnShow) { //...show the current altitude top-right
    char* trueAltitudeReadout = displayNumber(roundNumber(gTrueAltitudeDouble, cTrueAltitudeRoundToNearestFt));
    sprintf(gDisplayTopRightContent, "%6s" cFtLabel, trueAltitudeReadout);
    delete trueAltitudeReadout;
  }
  else {
    sprintf(gDisplayTopRightContent, "%c", ' ');
  }

  //Selected Altitude
  long tempSelectedAltitude = gSelectedAltitudeLong;
  char* selectedAltitudeReadout = displayNumber(tempSelectedAltitude);
  sprintf(gDisplayBottomRightContent, "%6s", selectedAltitudeReadout);
  delete selectedAltitudeReadout;


  if (gOledDim) {
    gOled.ssd1306_command(SSD1306_SETCONTRAST); //0x81
    gOled.ssd1306_command(0);
    
    gOled.ssd1306_command(SSD1306_SETPRECHARGE); //0xD9
    gOled.ssd1306_command(0);

    gOled.ssd1306_command(SSD1306_SETVCOMDETECT); //0xDB
    gOled.ssd1306_command(0);
  }
  else {
    gOled.ssd1306_command(SSD1306_SETCONTRAST); //0x81
    gOled.ssd1306_command(255);
    
    gOled.ssd1306_command(SSD1306_SETPRECHARGE); //0xD9
    gOled.ssd1306_command(255);

    gOled.ssd1306_command(SSD1306_SETVCOMDETECT); //0xDB
    gOled.ssd1306_command(255);
  }
  gOled.clearDisplay();
  gOled.setRotation(2);
  gOled.setTextColor(SSD1306_WHITE);
  gOled.setTextSize(1);
  gOled.setCursor(0, 0);
  gOled.println(gDisplayTopRightContent);
  gOled.setTextSize(3);
  gOled.setCursor(0, 11);
  gOled.println(gDisplayBottomRightContent);
  gOled.setTextSize(2);
  gOled.setCursor(25, 11);
  gOled.print(cFtLabel);
  gOled.display();
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
  if (altimeterSetting != gAltimeterSettingInHgDouble) {
    altimeterSetting = gAltimeterSettingInHgDouble; //this silences a compiler warning
    EEPROM.put(eepromIndex, altimeterSetting);
  }
  eepromIndex += sizeof(double);

  //Last altitude offset
  int altitudeOffset;
  EEPROM.get(eepromIndex, altitudeOffset);
  if (altitudeOffset != gCalibratedAltitudeOffsetInt) {
    altitudeOffset = gCalibratedAltitudeOffsetInt; //this silences a compiler warning
    EEPROM.put(eepromIndex, altitudeOffset);
  }
  eepromIndex += sizeof(int);

  
  //Semi-Permanent altitude offset
  EEPROM.get(eepromIndex, altitudeOffset);
  if (altitudeOffset != gPermanentCalibratedAltitudeOffsetInt) {
    altitudeOffset = gPermanentCalibratedAltitudeOffsetInt; //this silences a compiler warning
    EEPROM.put(eepromIndex, altitudeOffset);
  }
  eepromIndex += sizeof(int);

  //Screen dim
  bool dim;
  EEPROM.get(eepromIndex, dim);
  if (dim != gOledDim) {
    dim = gOledDim; //this silence a compiler warning
    EEPROM.put(eepromIndex, dim);
  }
  eepromIndex += sizeof(bool);

  //Sensor Mode - On/Show, On/Hide, Off
  int sensorMode;
  EEPROM.get(eepromIndex, sensorMode);
  if (sensorMode != static_cast<int>(gSensorMode)) {
    sensorMode = static_cast<int>(gSensorMode);
    EEPROM.put(eepromIndex, sensorMode);
  }
  eepromIndex += sizeof(int);

  //Altitude Units
  int altitudeUnits;
  EEPROM.get(eepromIndex, altitudeUnits);
  if (altitudeUnits != static_cast<int>(gAltitudeUnits)) {
    altitudeUnits = static_cast<int>(gAltitudeUnits);
    EEPROM.put(eepromIndex, altitudeUnits);
  }
  eepromIndex += sizeof(int);

  //Pressure Units
  int pressureUnits;
  EEPROM.get(eepromIndex, pressureUnits);
  if (pressureUnits != static_cast<int>(gPressureUnits)) {
    pressureUnits = static_cast<int>(gPressureUnits);
    EEPROM.put(eepromIndex, pressureUnits);
  }
  eepromIndex += sizeof(int);
}

//////////////////////////////////////////////////////////////////////////
double altitudeCorrected(double pressureAltitude) {
  return (pressureAltitude - (1 - (pow(gAltimeterSettingInHgDouble / cSeaLevelPressureInHg, 0.190284))) * 145366.45) + gCalibratedAltitudeOffsetInt + gPermanentCalibratedAltitudeOffsetInt;
}

//////////////////////////////////////////////////////////////////////////
// Prints number with commas, but only supports [0, 999999]
//////////////////////////////////////////////////////////////////////////
char* displayNumber(const long &number) {
  int thousands = static_cast<int>(number / 1000);
  int ones = static_cast<int>(number % 1000);
  char* result = new char[8];

  if (number >= 1000) {
    sprintf(result, "%01d,%03d", thousands, ones);
  }
  else if (number <= -1000) {
    sprintf(result, "%01d,%03d", thousands, abs(ones));
  }
  else {
    sprintf(result, "%01d", ones);
  }

  return result;
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
void println(String msg) {
  #ifdef DEBUG
  Serial.println(msg);
  #endif
}
