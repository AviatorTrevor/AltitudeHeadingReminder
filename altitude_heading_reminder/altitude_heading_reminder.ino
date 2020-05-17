/*
Author: Trevor Bartlett
Email: aviatortrevor@gmail.com
Cell: (760) 832-3480

Hardware device to remind pilots of assigned headings & altitudes. Also has buzzer feature
to alert the pilot of when he/she is approaching altitude, or departed from it.

*TODO:
*silence alarm for a second or two after it triggers
*implement screen brightness control
*flash screen for alerts
*add settings to disable 200ft and 1000ft alarms
*test sleeping
*test recovering pressure sensor?
*add more data fields to settings page
*interrupts causing an interrupt to beeping noises
*design for battery
*software for battery level & charging
*create software license - credit for libraries used
*     https://forum.arduino.cc/index.php?topic=175511.0
*     http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
*     https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
*buy louder buzzer? or add 2nd buzzer? Try different frequencies using
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
#include <LiquidCrystal_I2C.h>
//TODO #include <Adafruit_GFX.h>
//TODO #include <Adafruit_SSD1306.h> //TODO implement your own graphics libraries so that we can have a slim version to cut down on program storage space

#define DEBUG

#define cAppVersion                    "1.0.0"
#define cOneSecond                     1000 //1000 milliseconds = 1 second
#define cOneSecondBeforeOverflow       (unsigned long)(pow(2, sizeof(unsigned long) * 8) - cOneSecond)
#define cTenSeconds                    10 * cOneSecond
#define cFeetInMeters                  3.28084
#define cSeaLevelPressureMb            1013.25 //standard sea level pressure in millibars
#define cSeaLevelPressureInHg          29.92
#define cFtLabel                       "ft"
#define cInLabel                       "\""
#define cMbLabel                       "mb"
#define cMetersLabel                   "m"
#define cAltitudeSelectIncrement       100   //ft
#define cAltitudeFineSelectIncrement   10    //ft
#define cInitialSelectedAltitudeOffset 2000  //ft
#define cAltitudeHighSelectIncrement   1000  //ft
#define cHighAltitude                  18000 //ft
#define cLowestAltitudeSelect          -1500 //ft
#define cHighestAltitudeSelect         60000 //ft
#define cHighestAltitudeAlert          20000 //ft, the pressure sensor will only measure so high. No point in alerting above a certain pressure level
#define cAltimeterSettingInHgMin       27.50 //inHg
#define cAltimeterSettingInHgMax       31.50 //inHg
#define cAltimeterSettingInHgInterval  0.01  //inHg
#define cCalibrationOffsetMin          -990  //ft
#define cCalibrationOffsetMax          990   //ft
#define cCalibrationOffsetInterval     10    //ft
#define cHeadingSelectIncrement        5     //degrees
#define cTrueAltitudeRoundToNearest    10    //ft
#define cTryToRecover                  false

//EEPROM
#define         cEepromWriteDelay      10000  //milliseconds
volatile bool   gNeedToWriteToEeprom;

//Main program variables
double          gTrueAltitudeDouble;           //feet, true altitude
volatile long   gSelectedAltitudeLong; //feet
volatile double gAltimeterSettingInHgDouble = cSeaLevelPressureInHg; //inches of mercury
volatile int    gCalibratedAltitudeOffsetInt;
volatile int    gPermanentCalibratedAltitudeOffsetInt;
volatile int    gSelectedHeadingInt = 360; //degrees
enum AltitudeUnits {AltitudeUnitsFeet, AltitudeUnitsMeters, cNumberOfAltitudeUnits};
enum PressureUnits {PressureUnitsInHg, PressureUnitsMb, cNumberOfPressureUnits};
volatile AltitudeUnits gAltitudeUnits = AltitudeUnitsFeet;
volatile PressureUnits gPressureUnits = PressureUnitsInHg;

//Buzzer
#define            cInitialSelectedAltitudeTimeout 5000 //determine initial selection altitude within 5 seconds, or give up and allow it to stay at 0 because pressure sensor failed
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
enum BuzzAlarmMode {Climbing1000ToGo, Climbing200ToGo, Descending1000ToGo, Descending200ToGo, AltitudeDeviate, UrgentAlarm, AlarmDisabled, DetermineAlarmState};
BuzzAlarmMode      gAlarmModeEnum = AlarmDisabled;
BuzzAlarmMode      gNextAlarmModeEnum; //only used to get out of a multiple-beep alarm state
int                gBuzzCountInt; //always a value between [0, cUrgentBuzzNumberOfBeeps]

//Timing control
unsigned long          gNextSensorBeginCycleTs;
unsigned long          gNextSensorReadyTs;
unsigned long          gNextScreenRefreshTs; //TODO is this being used?
unsigned long          gNextBuzzTs;
volatile unsigned long gLeftButtonPressedTs;
volatile unsigned long gRightButtonPressedTs;
volatile unsigned long gLastRightRotaryActionTs;
volatile unsigned long gLeftRotaryReleaseTs;
volatile unsigned long gRightRotaryReleaseTs;
volatile unsigned long gEepromSaveNeededTs;
volatile unsigned long gLastRotaryActionTs;

//Display
#define cScreenBrightnessSettings 5 //TODO tune this to the new screen
#define cFlashCursorOnPeriod      800
#define cFlashCursorOffPeriod     200
#define cSplashScreenDelay        cOneSecond
#define cMaxScreenRefreshRate     30 //30Hz //TODO
#define cMaxScreenRefreshPeriod   (cOneSecond / cMaxScreenRefreshRate)
#define cDisplayAddr              0x27//TODO small display address is 0x3C
#define cScreenWidth              128 // OLED display width, in pixels
#define cScreenHeight             32  // OLED display height, in pixels
#define cOledReset                4   // Reset pin # (or -1 if sharing Arduino reset pin)
#define cLedBrightnessPin         11
//TODO small OLED display Adafruit_SSD1306 gDisplay(cScreenWidth, cScreenHeight, &Wire, cOledReset);
LiquidCrystal_I2C gDisplay(cDisplayAddr, 16, 2); //16x2 character display
volatile int gScreenBrightnessInt = cScreenBrightnessSettings; //initialize to brightest setting

//Cursor control
enum Cursor {
    CursorSelectHeading,
    CursorSelectAltimeter,
    CursorSelectOffset,
    CursorSelectBrightness,
    CursorSelectSensor,
    CursorSelectAltitudeUnits,
    CursorSelectPressureUnits,
    CursorViewSoftwareVersion,
    CursorViewBatteryLevel,
    cNumberOfCursorModes }
    gCursor = CursorSelectHeading;
    //TODO: set volume?
    //TODO: set tones?
    //TODO: turn alerts on/off?
    //TODO: current temperature?

//BMP180 Sensor variables
#define    cSensorLoopCycle               2 //2Hz
#define    cSensorLoopPeriod              (cOneSecond / cSensorLoopCycle)
#define    cBmp180Quality                 3 //highest quality, more electrical draw [0-3]. Wait times are {5ms, 8ms, 14ms, 26ms}. Getting temperature is always 5ms.
SFE_BMP180 gSensor;
char       gSensorStatusChar;             //byte value BMP180 pressure sensor returns. Acts as success status as well as time to wait for next state
int        gSensorProcessStateInt;        //0 = start measuring temperature, 1 = get temperature, 2 = start measuring pressure, 3 = get pressure
double     gSensorTemperatureDouble;      //celcius
double     gSensorPressureDouble;         //millibars
enum SensorMode {SensorModeOnShow, SensorModeOnHide, SensorModeOff, cNumberOfSensorModes};
volatile SensorMode gSensorMode = SensorModeOnShow;

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

  initializeDisplayDevice();
  initializeValuesFromEeprom();
  initializeBmp180Sensor();
  initializeRotaryKnobs();
  initializeBuzzer();
}

//////////////////////////////////////////////////////////////////////////
void initializeValuesFromEeprom() {
  int eepromIndex = 0;
  double tempDouble;
  int tempInt;
  
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
  EEPROM.get(eepromIndex, tempInt);
  gScreenBrightnessInt = constrain(tempInt, 1, cScreenBrightnessSettings);
  //TODO analogWrite(cLedBrightnessPin,(gScreenBrightnessInt - 1) * 49 + 59);
  eepromIndex += sizeof(int);


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
void initializeBmp180Sensor() {
  delay(300);
  if (eBMP180Failed = !gSensor.begin()) {
    eErrorCode = cBMP180InitFail;
  }
}

//////////////////////////////////////////////////////////////////////////
void initializeDisplayDevice() {
  /*if(eDisplayError = !gDisplay.begin(SSD1306_SWITCHCAPVCC, cDisplayAddr)) {
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
  }TODO OLED display*/

  gDisplay.init();
  gDisplay.clear();
  delay(50);
  gDisplay.backlight();
/*
  gDisplay.setCursor(4, 0);
  gDisplay.print("POWER ON");
  delay(1500);*/
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
    gNextSensorBeginCycleTs = gNextSensorReadyTs = gNextScreenRefreshTs = gNextBuzzTs = 0; //reset timing
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

  if (gCursor != CursorSelectHeading && gCursor != CursorSelectAltimeter && millis() - gLastRotaryActionTs >= cTenSeconds) {
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
    
    case CursorSelectOffset:
      gCalibratedAltitudeOffsetInt = constrain(roundNumber(gCalibratedAltitudeOffsetInt + cCalibrationOffsetInterval * increment, cCalibrationOffsetInterval), cCalibrationOffsetMin, cCalibrationOffsetMax);
      gEepromSaveNeededTs = millis();
      gNeedToWriteToEeprom = true;
      break;

    case CursorSelectBrightness:
      gScreenBrightnessInt = constrain(gScreenBrightnessInt + increment, 1, cScreenBrightnessSettings);
      //TODO analogWrite(cLedBrightnessPin,(gScreenBrightnessInt - 1) * 49 + 59);
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
      
    case CursorViewSoftwareVersion:
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
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm1000ToGo) {
        tone(cBuzzPin, cBuzzFrequencyA, cLongBuzzDuration);
        gAlarmModeEnum = Climbing200ToGo;
      }
      break;
    
    case Climbing200ToGo:
      if (gTrueAltitudeDouble >= gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = UrgentAlarm;
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
        gAlarmModeEnum = UrgentAlarm;
        gNextAlarmModeEnum = AltitudeDeviate;
        gNextBuzzTs = millis(); //next buzz time is now
      }
      else if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm1000ToGo) {
        gAlarmModeEnum = Descending1000ToGo;
      }
      break;
    
    case AltitudeDeviate: //We're looking to sound the alarm if pilot deviates from his altitude he already reached
      if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm200ToGo || gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm200ToGo) {
        gAlarmModeEnum = UrgentAlarm; //initiate beeping the alarm on the next pass
        gNextBuzzTs = millis(); //next buzz time is now
        if (gTrueAltitudeDouble > gSelectedAltitudeLong + cAlarm200ToGo) {
          gNextAlarmModeEnum = Descending200ToGo;
        }
        else if (gTrueAltitudeDouble < gSelectedAltitudeLong - cAlarm200ToGo) {
          gNextAlarmModeEnum = Climbing200ToGo;
        }
      }
      break;
      
    case UrgentAlarm:
      if (gBuzzCountInt == 0) {
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
      else if (gSensorMode != SensorModeOff || gSelectedAltitudeLong <= cHighestAltitudeAlert && millis() - gLastRightRotaryActionTs >= cDisableAlarmKnobMovementTime) {
        gAlarmModeEnum = DetermineAlarmState;
      }
      break;

    default: //default case
    case DetermineAlarmState:
      long diffBetweenSelectionAndTrueAltitude = gSelectedAltitudeLong - gTrueAltitudeDouble;
      if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || eBMP180Failed || eDisplayError) {
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
  //show the parameter name top-left and the associated value bottom-left
  char topLeftContent[8];
  char bottomLeftContent[8];
  char topRightContent[9];
  char bottomRightContent[9];
  
  switch (gCursor) {
    case CursorSelectHeading: //Display Selected Heading
    {
      sprintf(topLeftContent, "%s", "Hdg");
      sprintf(bottomLeftContent, "%03d%c", gSelectedHeadingInt, (char)(223)); //223 == degree symbol
      break;
    }

    case CursorSelectAltimeter:
      sprintf(topLeftContent, "%-s", "Altmtr");
      if (gPressureUnits == PressureUnitsInHg) {
        sprintf(bottomLeftContent, "%s%s", String(gAltimeterSettingInHgDouble).c_str(), cInLabel);
      }
      else { //(gPressureUnits == PressureUnitsMb) {
        sprintf(bottomLeftContent, "%s%s", String(static_cast<int>(gAltimeterSettingInHgDouble * cSeaLevelPressureMb/cSeaLevelPressureInHg)).c_str(), cMbLabel);
      }
      break;

    case CursorSelectOffset:
      sprintf(topLeftContent, "%-s", "Offset");
      sprintf(bottomLeftContent, "%+d%s", gCalibratedAltitudeOffsetInt, cFtLabel);
      break;

    case CursorSelectBrightness:
      sprintf(topLeftContent, "%s", "Brtness");
      sprintf(bottomLeftContent, "%-7d", gScreenBrightnessInt);
      break;

    case CursorSelectSensor:
      sprintf(topLeftContent, "%s", "Sensor");
      if (gSensorMode == SensorModeOnShow) {
        sprintf(bottomLeftContent, "%-7s", "ON/SHOW");
      }
      else if (gSensorMode == SensorModeOnHide) {
        sprintf(bottomLeftContent, "%-7s", "ON/HIDE");
      }
      else {
        sprintf(bottomLeftContent, "%-7s", "OFF");
      }
      break;

    case CursorSelectAltitudeUnits:
      sprintf(topLeftContent, "%-7s", "A Units");
      if (gAltitudeUnits == AltitudeUnitsFeet) {
        sprintf(bottomLeftContent, "%-7s", "Ft");
      }
      else { //(gAltitudeUnits == AltitudeUnitsMeters) {
        sprintf(bottomLeftContent, "%-7s", "Meters");
      }
      break;

    case CursorSelectPressureUnits:
      sprintf(topLeftContent, "%-7s", "P Units");
      if (gPressureUnits == PressureUnitsInHg) {
        sprintf(bottomLeftContent, "%-7s", "\"Hg");
      }
      else { //(gPressureUnits == PressureUnitsMb) {
        sprintf(bottomLeftContent, "%-7s", "mb");
      }
      break;

    case CursorViewSoftwareVersion:
      sprintf(topLeftContent, "%-7s", "Ver");
      sprintf(bottomLeftContent, "%-7s", cAppVersion);
      break;

    case CursorViewBatteryLevel:
      sprintf(topLeftContent, "%-7s", "Batt");
      sprintf(bottomLeftContent, "%-7s", "100%"); //TODO implement battery level
      break;
  }


  //show the sensor true altitude
  if (eBMP180Failed) { //if there's a sensor error, the top line should be the error message
    sprintf(topRightContent, "%s", "FAIL");
  }
  else if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || gTrueAltitudeDouble > cHighestAltitudeAlert + cAlarm200ToGo) {
    sprintf(topRightContent, "%s", "OFF");
  }
  else if (gSensorMode == SensorModeOnShow) { //...show the current altitude top-right
    if (gAltitudeUnits == AltitudeUnitsFeet) {
      sprintf(topRightContent, "%s%s", displayNumber(roundNumber(gTrueAltitudeDouble, cTrueAltitudeRoundToNearest)).c_str(), cFtLabel);
    }
    else if (gAltitudeUnits == AltitudeUnitsMeters) {
      sprintf(topRightContent, "%s%s", displayNumber(roundNumber(gTrueAltitudeDouble / cFeetInMeters, cTrueAltitudeRoundToNearest)).c_str(), cMetersLabel);
    }
  }
  else {
    sprintf(topRightContent, "%c", ' ');
  }

  //Selected Altitude
  long tempSelectedAltitude = gSelectedAltitudeLong;
  if (gAltitudeUnits == AltitudeUnitsFeet) {
    sprintf(bottomRightContent, "%s%s", displayNumber(tempSelectedAltitude).c_str(), cFtLabel);
  }
  else if (gAltitudeUnits == AltitudeUnitsMeters) {
    sprintf(bottomRightContent, "%s%s", displayNumber(tempSelectedAltitude / cFeetInMeters).c_str(), cMetersLabel);
  }
  
  
  //Update the display
  char topLine[17];
  char bottomLine[17];
  sprintf(topLine, "%-7s %8s", topLeftContent, topRightContent);
  sprintf(bottomLine, "%-7s %8s", bottomLeftContent, bottomRightContent);

  gDisplay.setCursor(0, 0);
  gDisplay.print(topLine);
  gDisplay.setCursor(0, 1);
  gDisplay.print(bottomLine);
  
  





















  //TODO: smaller OLED display code
  //If we're having display issues, try to recover
  /*if (eDisplayError && cTryToRecover) {
    initializeDisplayDevice();
    if (eDisplayError) { //if we're still having trouble, return
      return;
    }
  }


  gDisplay.invertDisplay(false); //TODO remove?
  gDisplay.clearDisplay();
  gDisplay.setTextSize(1);

  //show the parameter name top-left and the associated value bottom-left
  gDisplay.setCursor(0, 0);
  gDisplay.setTextSize(1);
  switch (gCursor) {
    case CursorSelectHeading: //Display Selected Heading
    {
      gDisplay.println("Heading");
      gDisplay.setCursor(0, 15);
      int tempSelectedHeading = gSelectedHeadingInt;
      char heading[4];
      sprintf(heading, "%03d%c", tempSelectedHeading, (char)(247)); //247 == degree symbol
      gDisplay.println(String(heading));
      break;
    }

    case CursorSelectAltimeter:
      gDisplay.println("Altimeter");
      gDisplay.setCursor(0, 15);
      gDisplay.println(String(gAltimeterSettingInHgDouble) + cInLabel);
      break;

    case CursorSelectOffset:
      gDisplay.println("Offset");
      gDisplay.setCursor(0, 15);
      if (gCalibratedAltitudeOffsetInt > 0) {
        gDisplay.println("+" + String(gCalibratedAltitudeOffsetInt) + cFtLabel);
      }
      else {
        gDisplay.println(String(gCalibratedAltitudeOffsetInt) + cFtLabel);
      }
      break;

    case CursorSelectBrightness:
      gDisplay.println("Brightness");
      gDisplay.setCursor(0, 15);
      gDisplay.println(String(gScreenBrightnessInt));
      break;

    case CursorSelectSensor:
      gDisplay.println("Sensor");
      gDisplay.setCursor(0, 15);
      if (gSensorMode == SensorModeOnShow) {
        gDisplay.println("ON/SHOW");
      }
      else if (gSensorMode == SensorModeOnHide) {
        gDisplay.println("ON/HIDE");
      }
      else {
        gDisplay.println("OFF");
      }
      break;

    case CursorSelectAltitudeUnits:
      gDisplay.println("Alt Units");
      gDisplay.setCursor(0, 15);
      if (gAltitudeUnits == AltitudeUnitsFeet) {
        gDisplay.println("Ft");
      }
      else { //(gAltitudeUnits == AltitudeUnitsMeters) {
        gDisplay.println("Meters");
      }
      break;

    case CursorSelectPressureUnits:
      gDisplay.println("Press Units");
      gDisplay.setCursor(0, 15);
      if (gPressureUnits == PressureUnitsInHg) {
        gDisplay.println("\"Hg");
      }
      else { //(gPressureUnits == PressureUnitsMb) {
        gDisplay.println("mb");
      }
      break;

    case CursorViewSoftwareVersion:
      gDisplay.println("Version");
      gDisplay.setCursor(0, 15);
      gDisplay.println(cAppVersion);
      break;

    case CursorViewBatteryLevel:
      gDisplay.println("Battery");
      gDisplay.setCursor(0, 15);
      gDisplay.println("??%");
      break;
  }



  //show the sensor true altitude
  if (eBMP180Failed) { //if there's a sensor error, the top line should be the error message
    gDisplay.setCursor(62, 0);
    gDisplay.println("SENSOR FAIL"); //TODO: in the future, write the error code to EEPROM
  }
  else if (gSensorMode == SensorModeOff || gSelectedAltitudeLong > cHighestAltitudeAlert || gTrueAltitudeDouble > cHighestAltitudeAlert + cAlarm200ToGo) {
    gDisplay.setCursor(68, 0);
    gDisplay.println("SENSOR OFF");
  }
  else if (gSensorMode == SensorModeOnShow) { //...show the current altitude top-right
    gDisplay.setCursor(80, 0);
    gDisplay.println(displayNumber(roundNumber(gTrueAltitudeDouble, cTrueAltitudeRoundToNearest)) + cFtLabel);
  }

  //...Display Selected Altitude
  gDisplay.setTextSize(2);
  gDisplay.setCursor(32, 15);
  long temporarySelectedAltitude = gSelectedAltitudeLong;
  gDisplay.println(displayNumber(temporarySelectedAltitude) + cFtLabel);

  gDisplay.display();*/
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

  //Last Screen Brightness
  int brightness;
  EEPROM.get(eepromIndex, brightness);
  if (brightness != gScreenBrightnessInt) {
    brightness = gScreenBrightnessInt; //this silence a compiler warning
    EEPROM.put(eepromIndex, brightness);
  }
  eepromIndex += sizeof(int);

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
