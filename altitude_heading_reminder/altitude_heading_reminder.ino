/*
Author: Trevor Bartlett
Email: aviatortrevor@gmail.com

Hardware device to remind pilots of assigned headings & altitudes. Also has buzzer feature
to alert the pilot of when he/she is approaching altitude, or departed from it.

*TODO:
*implement dual OLED displays
  *multiplexer or alternative method
  *test flash/invert screen when alerting
*when timer is running, show the timer in small font on all left-hand screens
*recently changed gFlashScreen to gFlashRightScreen. Look carefully at every existing location and determine how gFlashLeftScreen will fit in there
*analyze gAlarmMode being set outside of the handleAlarm() function
*left-display, shows altitude countdown
*adjust pin mapping for PCB board layout
*add settings to disable 200ft and 1000ft and altitude departure alarms
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
*4100 & 4200 alternating?
*/
#include <EEPROM.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <Custom_GFX.h>
#include <Custom_SSD1306.h>

#define DEBUG //TODO remove

#define cAppVersion                    "1.0" //[HardwareConfigOrMajorRedesign].[SoftwareRelease]
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

//EEPROM
#define         cSizeOfEeprom                       EEPROM.length() //1024
#define         cEepromWriteDelay                   5000  //milliseconds
#define         cEepromMaxWrites                    85000
#define         cEepromNextAvailableSlot            12
#define         cEepromAltimeterAddr                14
#define         cEepromAltitudeOffsetAddr           16
#define         cEepromPermanentAltitutdeOffsetAddr 18
#define         cEepromSensorModeAddr               20
#define         cEepromScreenDimAddr                22
#define         cEepromScreenOrientationAddr        24
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
enum SensorMode {SensorModeOff, SensorModeOnHide, SensorModeOnShow, cNumberOfSensorModes};
volatile SensorMode gSensorMode = SensorModeOnShow;

//Main program variables
double          gTrueAltitudeDouble;
volatile long   gSelectedAltitudeLong;
volatile double gAltimeterSettingInHgDouble = cSeaLevelPressureInHg;
volatile int    gCalibratedAltitudeOffsetInt;
volatile int    gPermanentCalibratedAltitudeOffsetInt;
volatile int    gSelectedHeadingInt = 360; //degrees

//Anti-piracy
volatile int    gSelectAppCode = 0;
volatile int    gAppCodeSequence = 0;
bool            gLegitimate = true;

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
#define            cDisableAlarmAfterAlarmTime     1800
#define            cDisableBacklightTimePeriod     200
#define            cDisableBacklightPriorToAlarm   300
enum BuzzAlarmMode {Climbing1000ToGo, Climbing200ToGo, Descending1000ToGo, Descending200ToGo, AltitudeDeviate, UrgentAlarm, LongAlarm, AlarmDisabled, DetermineAlarmState};
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
#define cOledAddr        0x3C
#define cOledWidth       128
#define cOledHeight      32
#define cOledReset       4
#define cLabelTextSize   1
#define cLabelTextYpos   0
#define cReadoutTextSize 3
#define cReadoutTextYpos 11
#define cMaxScreenRefreshRate     30 //30Hz //TODO remove?
#define cMaxScreenRefreshPeriod   (cOneSecond / cMaxScreenRefreshRate) //TODO remove?
Custom_SSD1306 gOled(cOledWidth, cOledHeight, &Wire, cOledReset);
volatile bool gOledDim = false;
volatile bool gDeviceFlipped = false;
bool gFlashRightScreen = false;
bool gFlashLeftScreen = false;
volatile bool gShowLeftScreen = false; //TODO temporary, remove

char gDisplayTopContent[20];
char gDisplayBottomContent[10];

//Cursor control
enum Cursor {
    CursorSelectHeading,
    CursorSelectAltimeter,
    CursorSelectTimer,
    CursorSelectBrightness,
    CursorSelectOffset,
    CursorSelectSensor,
    CursorSelectFlipDevice,
    CursorViewBmpTemp,
    CursorViewBatteryLevel,
    cNumberOfCursorModes }
    gCursor = CursorSelectHeading;
    //TODO: set volume?
    //TODO: set tones?
    //TODO: turn alerts on/off?

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
  initializeRotaryKnobs();
  initializePiracyCheck();
  initializeValuesFromEeprom();
  initializeBmp180Sensor();
  initializeBuzzer();
  delay(1200); //delay for splash screen (to see version number)
}

//////////////////////////////////////////////////////////////////////////
void initializeValuesFromEeprom() {
  int eepromIndex;
  double tempDouble;
  int tempInt;
  byte tempByte;
  bool tempBool;

  //Last Altimeter Setting
  EEPROM.get(cEepromAltimeterAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempDouble);
    if (isnan(tempDouble) || tempDouble < cAltimeterSettingInHgMin || tempDouble > cAltimeterSettingInHgMax) {
      gAltimeterSettingInHgDouble = cSeaLevelPressureInHg;
    }
    else {
      gAltimeterSettingInHgDouble = tempDouble;
    }
  }
  
  //Last altitude offset
  EEPROM.get(cEepromAltitudeOffsetAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempInt);
    gCalibratedAltitudeOffsetInt = tempInt;
    if (gCalibratedAltitudeOffsetInt < cCalibrationOffsetMin || gCalibratedAltitudeOffsetInt > cCalibrationOffsetMax) {
      gCalibratedAltitudeOffsetInt = 0;
    }
  }

  //Semi-permanent altitude offset
  EEPROM.get(cEepromPermanentAltitutdeOffsetAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempInt);
    if (tempInt >= cCalibrationOffsetMin && tempInt <= cCalibrationOffsetMax) {
      gPermanentCalibratedAltitudeOffsetInt = tempInt;
    }
    else {
      gPermanentCalibratedAltitudeOffsetInt = 0;
    }
  }

  //Sensor Mode - On/Show, On/Hide, Off
  EEPROM.get(cEepromSensorModeAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempByte);
    gSensorMode = static_cast<SensorMode>(constrain(tempByte, 0, cNumberOfSensorModes));
  }

  //Screen Brightness Dim
  EEPROM.get(cEepromScreenDimAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempBool);
    gOledDim = tempBool;
  }

  //Screen Orientation
  EEPROM.get(cEepromScreenOrientationAddr, eepromIndex);
  if (eepromIndex >= 0 && eepromIndex < cSizeOfEeprom) {
    EEPROM.get(eepromIndex, tempBool);
    gDeviceFlipped = tempBool;
  }
}

//////////////////////////////////////////////////////////////////////////
void initializeDefaultEeprom() {
  EEPROM.put(cEepromNextAvailableSlot, 49);
  
  EEPROM.put(cEepromAltimeterAddr, 26);
  EEPROM.put(26, (double)cSeaLevelPressureInHg); //default value for altimeter
  EEPROM.put(26 + sizeof(double), 0);
  
  EEPROM.put(cEepromAltitudeOffsetAddr, 32);
  EEPROM.put(32, (int)0); //default value for altitude offset
  EEPROM.put(32 + sizeof(int), 0);
  
  EEPROM.put(cEepromPermanentAltitutdeOffsetAddr, 36);
  EEPROM.put(36, (int)0); //default value for permanent altitude offset
  EEPROM.put(36 + sizeof(int), 0);
  
  EEPROM.put(cEepromSensorModeAddr, 40);
  EEPROM.put(40, (byte)0); //default value for mode
  EEPROM.put(40 + sizeof(byte), 0);
  
  EEPROM.put(cEepromScreenDimAddr, 43);
  EEPROM.put(43, false);
  EEPROM.put(43 + sizeof(bool), 0);
  
  EEPROM.put(cEepromScreenOrientationAddr, 46);
  EEPROM.put(46, false);
  EEPROM.put(46 + sizeof(bool), 0);
}

//////////////////////////////////////////////////////////////////////////
void initializePiracyCheck() {
  int eepromIndex = 0;
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
  gOled.invert(true);
  gOled.setCursor(13,0);
  gOled.setTextSize(2);
  gOled.print("ERROR 69");
  gOled.display();
  delay(3000);
  int lastWrittenSequence = 0;
  int currentAppCodeSequence = 0;
  int selectAppCode = gSelectAppCode;
  eepromIndex = 0;
  while (currentAppCodeSequence <= cAppCodeNumberOfDigits) {
    selectAppCode = gSelectAppCode;
    currentAppCodeSequence = gAppCodeSequence;
    gOled.clearDisplay();
    gOled.setCursor(0,0);
    gOled.print(selectAppCode);
    gOled.display();
    if (currentAppCodeSequence > lastWrittenSequence) {
      EEPROM.put(eepromIndex, selectAppCode);
      lastWrittenSequence = currentAppCodeSequence;
      eepromIndex += sizeof(int);
      gSelectAppCode = 0;
      if (currentAppCodeSequence == cAppCodeNumberOfDigits) { //final digit entered
        while (true) {
          initializeDefaultEeprom();
          gOled.clearDisplay();
          gOled.setCursor(12,0);
          gOled.print("PLEASE");
          gOled.setCursor(12, 16);
          gOled.print("RESTART");
          gOled.display();
          delay(999999999);
        }
      }
    }
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
  gOled.setTextColor(SSD1306_WHITE);
  gOled.setTextSize(2);
  gOled.setCursor(15,0);
  gOled.print("Version"); //TODO change logic to display "Version" on left screen, "1.0.0" on right screen
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

  handleBuzzer();
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
void handleLeftRotary(int rotaryButton, int rotaryDt, int rotaryClk) {
  gShowLeftScreen = true;
  //The button being pressed can lead to 1 of 3 outcomes: {Short Press, Long Press, a rotation occuring before the long press time is reached}
  gLeftRotaryButton = digitalRead(rotaryButton); //read button state
  int leftRotaryDt = digitalRead(rotaryDt);
  int leftRotaryClk = digitalRead(rotaryClk);

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
      if (gOledDim) {
        gOledDim = false;
      }
      else {
        gOledDim = true;
      }
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
        gAlarmModeEnum = DetermineAlarmState;
      }
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;

    case CursorSelectFlipDevice:
      if (gDeviceFlipped) {
        gDeviceFlipped = false;
      }
      else {
        gDeviceFlipped = true;
      }
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;

    case CursorViewBmpTemp:
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
  else if (gSelectedHeadingInt == 111 && gSelectedAltitudeLong == -1500) { //magic numbers to reset EEPROM
    initializeDefaultEeprom();
  }
}

//////////////////////////////////////////////////////////////////////////
void handleRightRotary(int rotaryButton, int rotaryDt, int rotaryClk) {
  gShowLeftScreen = false;
  //The button being pressed on the right knob can only be used for fine-tuning mode or altitude-sync (long press). A released state indicates normal altitude selection mode.
  gRightRotaryButton = digitalRead(rotaryButton); //read button state
  int rightRotaryDt = digitalRead(rotaryDt);
  int rightRotaryClk = digitalRead(rotaryClk);

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

  gAlarmModeEnum = DetermineAlarmState; //disable alarm if we change selected altitude
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
  gAlarmModeEnum = DetermineAlarmState; //disable alarm

  if (gSensorMode == SensorModeOff) {
    return; //don't sync the altitude if we're not measuring the current altitude
  }

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
        gAlarmModeEnum = LongAlarm;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      break;
    }

    case Descending1000ToGo:
    {
      if (gTrueAltitudeDouble <= gSelectedAltitudeLong + cAlarm1000ToGo) {
        gAlarmModeEnum = LongAlarm;
        gNextBuzzTs = millis(); //next buzz time is now
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
          gFlashRightScreen = true;
          gNextBuzzTs = millis() + cShortBuzzOnFreqBDuration;
        }
        else if (gBuzzCountInt % 2 == 1) {
          tone(cBuzzPin, cBuzzFrequencyA);
          gFlashRightScreen = true;
          gNextBuzzTs = millis() + cShortBuzzOnFreqADuration;
        }
        else {
          gAlarmModeEnum = AlarmDisabled;
          noTone(cBuzzPin);
          gFlashRightScreen = false;
        }
      }
      break;
    }

    case LongAlarm:
      if (gBuzzCountInt == 0) {
        gLastAlarmTs = millis();
        gBuzzCountInt = 2;
      }
      if (millis() >= gNextBuzzTs) {
        if (gBuzzCountInt == 2) {
          gBuzzCountInt = 1;
          tone(cBuzzPin, cBuzzFrequencyA);
          gNextBuzzTs = millis() + cLongBuzzDuration;
          gFlashRightScreen = true;
        }
        else {
          gBuzzCountInt = 0;
          gAlarmModeEnum = AlarmDisabled;
          noTone(cBuzzPin);
          gFlashRightScreen = false;
        }
      }
      break;

    case AlarmDisabled:
    {
      if (millis() - gLastAlarmTs < cDisableAlarmAfterAlarmTime || eBMP180Failed) {
        //if alarm disabled or BMP180 failed, do nothing
      }
      else if (!gInitializedAltitude) {
        /* I don't want it to go into a mode where it can beep unless we've given 
         * the pressure sensor and arduino a chance to initialize the altitude,
         * otherwise you'll hear a beep right at start-up */
        if (millis() > 5000) {
          gInitializedAltitude = true;
        }
      }
      else if (gSensorMode != SensorModeOff && gSelectedAltitudeLong <= cHighestAltitudeAlert && millis() - gLastRightRotaryActionTs >= cDisableAlarmKnobMovementTime) {
        gAlarmModeEnum = DetermineAlarmState;
      }
      break;
    }

    default: //default case
    case DetermineAlarmState:
    {
      long diffBetweenSelectionAndTrueAltitude = gSelectedAltitudeLong - gTrueAltitudeDouble;
      if (millis() - gLastRightRotaryActionTs < cDisableAlarmKnobMovementTime) {
        noTone(cBuzzPin); //stop the buzzer
        gFlashRightScreen = false;
        gBuzzCountInt = 0;
      }
      else if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || eBMP180Failed) {
        gAlarmModeEnum = AlarmDisabled;
        noTone(cBuzzPin); //stop the buzzer
        gFlashRightScreen = false;
        gBuzzCountInt = 0;
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
  //TODO
  if (gShowLeftScreen) {
    drawLeftScreen();
  }
  else {
    drawRightScreen();
  }
}

//////////////////////////////////////////////////////////////////////////
void drawLeftScreen() {
  gOled.clearDisplay();
  if (gDeviceFlipped) {
    gOled.setRotation(2);
  }
  else {
    gOled.setRotation(0);
  }
  gOled.invertDisplay(gFlashLeftScreen);

  switch (gCursor) {
    case CursorSelectHeading: //Display Selected Heading
    {
      sprintf(gDisplayTopContent, "%s", "Heading");
      sprintf(gDisplayBottomContent, "%03d", gSelectedHeadingInt);
      gOled.setTextSize(2);
      gOled.setCursor(55, 11);
      gOled.print((char)(247)); //247 = degree symbol
      break;
    }

    case CursorSelectAltimeter:
      sprintf(gDisplayTopContent, "%s", "Altimeter");
      sprintf(gDisplayBottomContent, "%d.%02d" cInLabel, (int)gAltimeterSettingInHgDouble, (int)(gAltimeterSettingInHgDouble*100)%100);
      break;

    case CursorSelectTimer:
      sprintf(gDisplayTopContent, "%s", "Stopwatch");
      if (gTimerStartTs == 0) {
        sprintf(gDisplayBottomContent, "00:00");
      }
      else {
        unsigned long seconds = (millis() - gTimerStartTs) / 1000;
        unsigned long minutes = seconds / 60;
        while (minutes >= 100) {
          minutes -= 100;
        }
        seconds -= minutes * 60;
        sprintf(gDisplayBottomContent, "%02d:%02d", (int)minutes, (int)seconds);
      }
      break;

    case CursorSelectBrightness:
      sprintf(gDisplayTopContent, "%s", "Brightness");
      if (gOledDim) {
        sprintf(gDisplayBottomContent, "%s", "DIM");
      }
      else {
        sprintf(gDisplayBottomContent, "%s", "BRIGHT");
      }
      break;

    case CursorSelectOffset:
      sprintf(gDisplayTopContent, "%s", "Calibration");
      sprintf(gDisplayBottomContent, "%+d" cFtLabel, gCalibratedAltitudeOffsetInt);
      break;

    case CursorSelectSensor:
      sprintf(gDisplayTopContent, "%s", "Sensor");
      if (gSensorMode == SensorModeOnShow) {
        sprintf(gDisplayBottomContent, "%s", "ON/SHOW");
      }
      else if (gSensorMode == SensorModeOnHide) {
        sprintf(gDisplayBottomContent, "%s", "ON/HIDE");
      }
      else {
        sprintf(gDisplayBottomContent, "%s", "OFF");
      }
      break;

    case CursorSelectFlipDevice:
      sprintf(gDisplayTopContent, "%s", "Orientation");
      sprintf(gDisplayBottomContent, "UP%c", (char)(24));
      break;

    case CursorViewBmpTemp:
    {
      sprintf(gDisplayTopContent, "%s", "Temperature");
      double temperatureFarenheit = gSensorTemperatureDouble * 9 / 5 + 32;
      sprintf(gDisplayBottomContent, "%d.%d %c", (int)temperatureFarenheit, (int)(temperatureFarenheit*10)%10, cDegFLabel);
      gOled.setTextSize(2);
      if (temperatureFarenheit >= 100) {
        gOled.setCursor(93, 11);
      }
      else {
        gOled.setCursor(75, 11);
      }
      gOled.print((char)(247));
      break;
    }

    case CursorViewBatteryLevel:
      sprintf(gDisplayTopContent, "%s", "Battery");
      sprintf(gDisplayBottomContent, "%s", "100%"); //TODO implement battery level
      break;
  }

  if (gOledDim) {
    gOled.dim(true, 0);
  }
  else {
    gOled.dim(false, 255);
  }
  
  gOled.setTextSize(cLabelTextSize);
  gOled.setCursor(0, cLabelTextYpos);
  gOled.print(gDisplayTopContent);

  gOled.setTextSize(cReadoutTextSize);
  gOled.setCursor(0, cReadoutTextYpos);
  gOled.print(gDisplayBottomContent);
  
  gOled.display();
}

//////////////////////////////////////////////////////////////////////////
void drawRightScreen() {
  gOled.clearDisplay();
  if (gDeviceFlipped) {
    gOled.setRotation(2);
  }
  else {
    gOled.setRotation(0);
  }
  gOled.invertDisplay(gFlashRightScreen);

  //show the sensor true altitude
  if (eBMP180Failed) { //if there's a sensor error, the top line should be the error message
    sprintf(gDisplayTopContent, "%6s", "FAIL");
  }
  else if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || gTrueAltitudeDouble > cHighestAltitudeAlert + cAlarm200ToGo) {
    sprintf(gDisplayTopContent, "%6s", "OFF");
  }
  else if (gSensorMode == SensorModeOnShow) { //...show the current altitude top-right
    char* trueAltitudeReadout = displayNumber(roundNumber(gTrueAltitudeDouble, cTrueAltitudeRoundToNearestFt));
    sprintf(gDisplayTopContent, "%6s", trueAltitudeReadout);
    delete trueAltitudeReadout;
    gOled.setTextSize(cLabelTextSize);
    gOled.setCursor(106, cLabelTextYpos);
    gOled.print(cFtLabel);
  }
  else {
    sprintf(gDisplayTopContent, "%c", ' ');
  }

  //Selected Altitude
  long tempSelectedAltitude = gSelectedAltitudeLong;
  char* selectedAltitudeReadout = displayNumber(tempSelectedAltitude);
  sprintf(gDisplayBottomContent, "%6s", selectedAltitudeReadout);
  delete selectedAltitudeReadout;


  if (gOledDim) {
    gOled.dim(true, 0);
  }
  else {
    gOled.dim(false, 255);
  }
  
  gOled.setTextSize(cLabelTextSize);
  gOled.setCursor(70, cLabelTextYpos);
  gOled.print(gDisplayTopContent);

  gOled.setTextSize(cReadoutTextSize);
  gOled.setCursor(0, cReadoutTextYpos);
  gOled.print(gDisplayBottomContent);

  gOled.setTextSize(2);
  gOled.setCursor(104, 18);
  gOled.print(cFtLabel);
  
  gOled.display();
}


//////////////////////////////////////////////////////////////////////////
ISR (PCINT0_vect) {    // handle pin change interrupt for D8 to D13 here
  if (gDeviceFlipped) {
    handleLeftRotary(cPinRightRotaryButton, cPinRightRotarySignalDt, cPinRightRotarySignalClk);
  }
  else {
    handleRightRotary(cPinRightRotaryButton, cPinRightRotarySignalDt, cPinRightRotarySignalClk);
  }
}

//////////////////////////////////////////////////////////////////////////
ISR (PCINT2_vect) {    // handle pin change interrupt for D0 to D7 here
  if (gDeviceFlipped) {
    handleRightRotary(cPinLeftRotaryButton, cPinLeftRotarySignalDt, cPinLeftRotarySignalClk);
  }
  else {
    handleLeftRotary(cPinLeftRotaryButton, cPinLeftRotarySignalDt, cPinLeftRotarySignalClk);
  }
}

//////////////////////////////////////////////////////////////////////////
// we only write data to EEPROM if the value changes, because EEPROM has
// a limited number of writes, but unlimited reads.
// we maximize the usage of the EEPROM by moving the data to a new block
// once we're nearing the limit of that EEPROM's block max writes
//
// byte 0-1           app code 1
// byte 2-3           app code 2
// byte 4-6           app code 3
// byte 6-7           app code 4
// byte 8-9           app code 5
// byte 10-11         app code 6
// byte 12-13         next available block
// byte 14-15         address of altimeter
// byte 16-17         address of altitude offset
// byte 18-19         address of permanenent offset
// byte 20-21         address of sensor mode
// byte 22-23         address of screen dim
// byte 24-25         address of screen orientation
// byte 26-29         original altimeter value
// byte 30-31         original altimeter EEPROM-write-counter
// byte 32-33         original altitude offset value
// byte 34-35         original altitude offset EEPROM-write-counter
// byte 36-37         original permanent altitude offset value
// byte 38-39         original permanent altitude offset EEPROM-write-counter
// byte 40            original sensor mode value
// byte 41-42         original sensor mode EEPROM-write-counter
// byte 43            original screen dim value
// byte 44-45         original screen dim EEPROM-write-counter
// byte 46            original screen orientation value
// byte 47-48         original screen orientation EEPROM-write-counter
//////////////////////////////////////////////////////////////////////////
void writeValuesToEeprom() {
  gNeedToWriteToEeprom = false;
  int eepromIndex;
  int numOfWrites;
  int datatypeSize;
  int dataAddr;
  
  //Last Altimeter Setting
  datatypeSize = sizeof(double);
  dataAddr = cEepromAltimeterAddr;
  double altimeterSetting;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, altimeterSetting);
  if (altimeterSetting != gAltimeterSettingInHgDouble) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }
    altimeterSetting = gAltimeterSettingInHgDouble; //this silences a compiler warning
    EEPROM.put(eepromIndex, altimeterSetting);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }


  //Last altitude offset
  datatypeSize = sizeof(int);
  dataAddr = cEepromAltitudeOffsetAddr;
  int altitudeOffset;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, altitudeOffset);
  if (altitudeOffset != gCalibratedAltitudeOffsetInt) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }
    altitudeOffset = gCalibratedAltitudeOffsetInt; //this silences a compiler warning
    EEPROM.put(eepromIndex, altitudeOffset);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }

  
  //Semi-Permanent altitude offset
  datatypeSize = sizeof(int);
  dataAddr = cEepromPermanentAltitutdeOffsetAddr;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, altitudeOffset);
  if (altitudeOffset != gPermanentCalibratedAltitudeOffsetInt) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }
    altitudeOffset = gPermanentCalibratedAltitudeOffsetInt; //this silences a compiler warning
    EEPROM.put(eepromIndex, altitudeOffset);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }


  //Sensor Mode - On/Show, On/Hide, Off
  datatypeSize = sizeof(byte);
  dataAddr = cEepromSensorModeAddr;
  byte sensorMode;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, sensorMode);
  if (sensorMode != static_cast<byte>(gSensorMode)) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }
    sensorMode = static_cast<byte>(gSensorMode); //this silences a compiler warning
    EEPROM.put(eepromIndex, sensorMode);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }


  //Screen dim
  datatypeSize = sizeof(bool);
  dataAddr = cEepromScreenDimAddr;
  bool dim;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, dim);
  if (dim != gOledDim) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }

    dim = gOledDim; //this silences a compiler warning
    EEPROM.put(eepromIndex, dim);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }


  //Screen orientation
  datatypeSize = sizeof(bool);
  dataAddr = cEepromScreenOrientationAddr;
  bool screenFlip;
  EEPROM.get(dataAddr, eepromIndex);
  EEPROM.get(eepromIndex, screenFlip);
  if (screenFlip != gDeviceFlipped) {
    EEPROM.get(eepromIndex + datatypeSize, numOfWrites);
    if (numOfWrites > cEepromMaxWrites) {
      EEPROM.get(cEepromNextAvailableSlot, eepromIndex);
      EEPROM.put(eepromIndex + datatypeSize, 0); //reset write counter
      EEPROM.put(dataAddr, eepromIndex); //reset address
      EEPROM.put(cEepromNextAvailableSlot, eepromIndex + datatypeSize + sizeof(int)); //reset next available slot
    }
  
    screenFlip = gDeviceFlipped; //this silences a compiler warning
    EEPROM.put(eepromIndex, screenFlip);
    EEPROM.put(eepromIndex + datatypeSize, numOfWrites + 1);
  }
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
