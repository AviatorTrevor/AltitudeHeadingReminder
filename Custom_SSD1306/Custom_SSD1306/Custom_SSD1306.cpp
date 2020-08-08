/*!
 * @file Custom_SSD1306.cpp
 *
 * @mainpage Arduino library for monochrome OLEDs based on SSD1306 drivers.
 *
 * @section intro_sec Introduction
 *
 * This is documentation for Adafruit's SSD1306 library for monochrome
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * These displays use I2C or SPI to communicate. I2C requires 2 pins
 * (SCL+SDA) and optionally a RESET pin. SPI requires 4 pins (MOSI, SCK,
 * select, data/command) and optionally a reset pin. Hardware SPI or
 * 'bitbang' software SPI are both supported.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a href="https://github.com/adafruit/Adafruit-GFX-Library">
 * Custom_GFX</a> being present on your system. Please make sure you have
 * installed the latest version before using this library.
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text above, and the splash screen included below,
 * must be included in any redistribution.
 *
 */

#ifdef __AVR__
 #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) \
  (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
 #include <util/delay.h>
#endif

#include <Custom_GFX.h>
#include "Custom_SSD1306.h"

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#if defined(BUFFER_LENGTH)
 #define WIRE_MAX BUFFER_LENGTH          ///< AVR or similar Wire lib
#elif defined(SERIAL_BUFFER_SIZE)
 #define WIRE_MAX (SERIAL_BUFFER_SIZE-1) ///< Newer Wire uses RingBuffer
#else
 #define WIRE_MAX 32                     ///< Use common Arduino core default
#endif

#define ssd1306_swap(a, b) \
  (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

#if ARDUINO >= 100
 #define WIRE_WRITE wire->write ///< Wire write function in recent Arduino lib
#else
 #define WIRE_WRITE wire->send  ///< Wire write function in older Arduino lib
#endif

#ifdef HAVE_PORTREG
 #define SSD1306_SELECT       *csPort &= ~csPinMask; ///< Device select
 #define SSD1306_DESELECT     *csPort |=  csPinMask; ///< Device deselect
 #define SSD1306_MODE_COMMAND *dcPort &= ~dcPinMask; ///< Command mode
 #define SSD1306_MODE_DATA    *dcPort |=  dcPinMask; ///< Data mode
#else
 #define SSD1306_SELECT       digitalWrite(csPin, LOW);  ///< Device select
 #define SSD1306_DESELECT     digitalWrite(csPin, HIGH); ///< Device deselect
 #define SSD1306_MODE_COMMAND digitalWrite(dcPin, LOW);  ///< Command mode
 #define SSD1306_MODE_DATA    digitalWrite(dcPin, HIGH); ///< Data mode
#endif

#if (ARDUINO >= 157) && !defined(ARDUINO_STM32_FEATHER)
 #define SETWIRECLOCK wire->setClock(wireClk)    ///< Set before I2C transfer
 #define RESWIRECLOCK wire->setClock(restoreClk) ///< Restore after I2C xfer
#else // setClock() is not present in older Arduino Wire lib (or WICED)
 #define SETWIRECLOCK ///< Dummy stand-in define
 #define RESWIRECLOCK ///< keeps compiler happy
#endif

#if defined(SPI_HAS_TRANSACTION)
 #define SPI_TRANSACTION_START spi->beginTransaction(spiSettings) ///< Pre-SPI
 #define SPI_TRANSACTION_END   spi->endTransaction()              ///< Post-SPI
#else // SPI transactions likewise not present in older Arduino SPI lib
 #define SPI_TRANSACTION_START ///< Dummy stand-in define
 #define SPI_TRANSACTION_END   ///< keeps compiler happy
#endif

// The definition of 'transaction' is broadened a bit in the context of
// this library -- referring not just to SPI transactions (if supported
// in the version of the SPI library being used), but also chip select
// (if SPI is being used, whether hardware or soft), and also to the
// beginning and end of I2C transfers (the Wire clock may be sped up before
// issuing data to the display, then restored to the default rate afterward
// so other I2C device types still work).  All of these are encapsulated
// in the TRANSACTION_* macros.

// Check first if Wire, then hardware SPI, then soft SPI:
#define TRANSACTION_START   SETWIRECLOCK;
#define TRANSACTION_END     RESWIRECLOCK;

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for I2C-interfaced SSD1306 displays.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  twi
            Pointer to an existing TwoWire instance (e.g. &Wire, the
            microcontroller's primary I2C bus).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  clkDuring
            Speed (in Hz) for Wire transmissions in SSD1306 library calls.
            Defaults to 400000 (400 KHz), a known 'safe' value for most
            microcontrollers, and meets the SSD1306 datasheet spec.
            Some systems can operate I2C faster (800 KHz for ESP32, 1 MHz
            for many other 32-bit MCUs), and some (perhaps not all)
            SSD1306's can work with this -- so it's optionally be specified
            here and is not a default behavior. (Ignored if using pre-1.5.7
            Arduino software, which operates I2C at a fixed 100 KHz.)
    @param  clkAfter
            Speed (in Hz) for Wire transmissions following SSD1306 library
            calls. Defaults to 100000 (100 KHz), the default Arduino Wire
            speed. This is done rather than leaving it at the 'during' speed
            because other devices on the I2C bus might not be compatible
            with the faster rate. (Ignored if using pre-1.5.7 Arduino
            software, which operates I2C at a fixed 100 KHz.)
    @return Custom_SSD1306 object.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Custom_SSD1306::Custom_SSD1306(uint8_t w, uint8_t h, TwoWire *twi,
  int8_t rst_pin, uint32_t clkDuring, uint32_t clkAfter) :
  Custom_GFX(w, h), spi(NULL), wire(twi ? twi : &Wire), buffer(NULL),
  mosiPin(-1), clkPin(-1), dcPin(-1), csPin(-1), rstPin(rst_pin)
#if ARDUINO >= 157
  , wireClk(clkDuring), restoreClk(clkAfter)
#endif
{
}


/*!
    @brief  Destructor for Custom_SSD1306 object.
*/
Custom_SSD1306::~Custom_SSD1306(void) {
  if(buffer) {
    free(buffer);
    buffer = NULL;
  }
}

// LOW-LEVEL UTILS ---------------------------------------------------------

// Issue single byte out SPI, either soft or hardware as appropriate.
// SPI transaction/selection must be performed in calling function.
inline void Custom_SSD1306::SPIwrite(uint8_t d) {
  if(spi) {
    (void)spi->transfer(d);
  } else {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef HAVE_PORTREG
      if(d & bit) *mosiPort |=  mosiPinMask;
      else        *mosiPort &= ~mosiPinMask;
      *clkPort |=  clkPinMask; // Clock high
      *clkPort &= ~clkPinMask; // Clock low
#else
      digitalWrite(mosiPin, d & bit);
      digitalWrite(clkPin , HIGH);
      digitalWrite(clkPin , LOW);
#endif
    }
  }
}

// Issue single command to SSD1306, using I2C or hard/soft SPI as needed.
// Because command calls are often grouped, SPI transaction and selection
// must be started/ended in calling function for efficiency.
// This is a private function, not exposed (see ssd1306_command() instead).
void Custom_SSD1306::ssd1306_command1(uint8_t c) {
  //if(wire) { // I2C. this if statement removed
  wire->beginTransmission(i2caddr);
  WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
  WIRE_WRITE(c);
  wire->endTransmission();
}

// Issue list of commands to SSD1306, same rules as above re: transactions.
// This is a private function, not exposed.
void Custom_SSD1306::ssd1306_commandList(const uint8_t *c, uint8_t n) {
  //if(wire) { // I2C. this if statement removed
  wire->beginTransmission(i2caddr);
  WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
  uint8_t bytesOut = 1;
  while(n--) {
    if(bytesOut >= WIRE_MAX) {
      wire->endTransmission();
      wire->beginTransmission(i2caddr);
      WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
      bytesOut = 1;
    }
    WIRE_WRITE(pgm_read_byte(c++));
    bytesOut++;
  }
  wire->endTransmission();
}

// A public version of ssd1306_command1(), for existing user code that
// might rely on that function. This encapsulates the command transfer
// in a transaction start/end, similar to old library's handling of it.
/*!
    @brief  Issue a single low-level command directly to the SSD1306
            display, bypassing the library.
    @param  c
            Command to issue (0x00 to 0xFF, see datasheet).
    @return None (void).
*/
void Custom_SSD1306::ssd1306_command(uint8_t c) {
  TRANSACTION_START
  ssd1306_command1(c);
  TRANSACTION_END
}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  vcs
            VCC selection. Pass SSD1306_SWITCHCAPVCC to generate the display
            voltage (step up) from the 3.3V source, or SSD1306_EXTERNALVCC
            otherwise. Most situations with Adafruit SSD1306 breakouts will
            want SSD1306_SWITCHCAPVCC.
    @param  addr
            I2C address of corresponding SSD1306 display (or pass 0 to use
            default of 0x3C for 128x32 display, 0x3D for all others).
            SPI displays (hardware or software) do not use addresses, but
            this argument is still required (pass 0 or any value really,
            it will simply be ignored). Default if unspecified is 0.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SSD1306 displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @param  periphBegin
            If true, and if a hardware peripheral is being used (I2C or SPI,
            but not software SPI), call that peripheral's begin() function,
            else (false) it has already been done in one's sketch code.
            Cases where false might be used include multiple displays or
            other devices sharing a common bus, or situations on some
            platforms where a nonstandard begin() function is available
            (e.g. a TwoWire interface on non-default pins, as can be done
            on the ESP8266 and perhaps others).
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
boolean Custom_SSD1306::begin(uint8_t vcs, uint8_t addr, boolean reset,
  boolean periphBegin) {

  if((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
    return false;

  clearDisplay();

  vccstate = vcs;

  // Setup pin directions
  //if(wire) { // Using I2C. this if statement was removed
  // If I2C address is unspecified, use default
  // (0x3C for 32-pixel-tall displays, 0x3D for all others).
  i2caddr = 0x3C;
  // TwoWire begin() function might be already performed by the calling
  // function if it has unusual circumstances (e.g. TWI variants that
  // can accept different SDA/SCL pins, or if two SSD1306 instances
  // with different addresses -- only a single begin() is needed).
  if(periphBegin) wire->begin();

  // Reset SSD1306 if requested and reset pin specified in constructor
  if(reset && (rstPin >= 0)) {
    pinMode(     rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(1);                   // VDD goes high at start, pause for 1 ms
    digitalWrite(rstPin, LOW);  // Bring reset low
    delay(10);                  // Wait 10 ms
    digitalWrite(rstPin, HIGH); // Bring out of reset
  }

  TRANSACTION_START

  // Init sequence
  static const uint8_t PROGMEM init1[] = {
    SSD1306_DISPLAYOFF,                   // 0xAE
    SSD1306_SETDISPLAYCLOCKDIV,           // 0xD5
    0x80,                                 // the suggested ratio 0x80
    SSD1306_SETMULTIPLEX };               // 0xA8
  ssd1306_commandList(init1, sizeof(init1));
  ssd1306_command1(HEIGHT - 1);

  static const uint8_t PROGMEM init2[] = {
    SSD1306_SETDISPLAYOFFSET,             // 0xD3
    0x0,                                  // no offset
    SSD1306_SETSTARTLINE | 0x0,           // line #0
    SSD1306_CHARGEPUMP };                 // 0x8D
  ssd1306_commandList(init2, sizeof(init2));

  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

  static const uint8_t PROGMEM init3[] = {
    SSD1306_MEMORYMODE,                   // 0x20
    0x00,                                 // 0x0 act like ks0108
    SSD1306_SEGREMAP | 0x1,
    SSD1306_COMSCANDEC };
  ssd1306_commandList(init3, sizeof(init3));

  uint8_t comPins = 0x02;
  contrast = 0x8F;

  //screen is 128x32
  comPins = 0x02;
  contrast = 0x8F;

  ssd1306_command1(SSD1306_SETCOMPINS);
  ssd1306_command1(comPins);
  ssd1306_command1(SSD1306_SETCONTRAST);
  ssd1306_command1(contrast);

  ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
  ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
  static const uint8_t PROGMEM init5[] = {
    SSD1306_SETVCOMDETECT,               // 0xDB
    0x40,
    SSD1306_DISPLAYALLON_RESUME,         // 0xA4
    SSD1306_NORMALDISPLAY,               // 0xA6
    SSD1306_DEACTIVATE_SCROLL,
    SSD1306_DISPLAYON };                 // Main screen turn on
  ssd1306_commandList(init5, sizeof(init5));

  TRANSACTION_END

  return true; // Success
}

/*!
    @brief  Set/clear/invert a single pixel. This is also invoked by the
            Adafruit_GFX library in generating many higher-level graphics
            primitives.
    @param  x
            Column of display -- 0 at left to (screen width - 1) at right.
    @param  y
            Row of display -- 0 at top to (screen height -1) at bottom.
    @param  color
            Pixel color, one of: SSD1306_BLACK, SSD1306_WHITE or SSD1306_INVERT.
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Custom_SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
    // Pixel is in-bounds. Rotate coordinates if needed.
    switch (getRotation()) {
    case 1:
      ssd1306_swap(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      ssd1306_swap(x, y);
      y = HEIGHT - y - 1;
      break;
    }
    switch (color) {
    case SSD1306_WHITE:
      buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
      break;
    case SSD1306_BLACK:
      buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
      break;
    case SSD1306_INVERSE:
      buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
      break;
    }
  }
}



/*!
    @brief  Clear contents of display buffer (set all pixels to off).
    @return None (void).
    @note   Changes buffer contents only, no immediate effect on display.
            Follow up with a call to display(), or with other graphics
            commands as needed by one's own application.
*/
void Custom_SSD1306::clearDisplay(void) {
  memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
}



// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
    @return None (void).
    @note   Drawing operations are not visible until this function is
            called. Call after each graphics command, or after a whole set
            of graphics commands, as best needed by one's own application.
*/
void Custom_SSD1306::display(void) {
  TRANSACTION_START
  static const uint8_t PROGMEM dlist1[] = {
    SSD1306_PAGEADDR,
    0,                         // Page start address
    0xFF,                      // Page end (not really, but works here)
    SSD1306_COLUMNADDR,
    0 };                       // Column start address
  ssd1306_commandList(dlist1, sizeof(dlist1));
  ssd1306_command1(WIDTH - 1); // Column end address

#if defined(ESP8266)
  // ESP8266 needs a periodic yield() call to avoid watchdog reset.
  // With the limited size of SSD1306 displays, and the fast bitrate
  // being used (1 MHz or more), I think one yield() immediately before
  // a screen write and one immediately after should cover it.  But if
  // not, if this becomes a problem, yields() might be added in the
  // 32-byte transfer condition below.
  yield();
#endif
  uint16_t count = WIDTH * ((HEIGHT + 7) / 8);
  uint8_t *ptr   = buffer;
  //if(wire) { // I2C. this if statement removed
  wire->beginTransmission(i2caddr);
  WIRE_WRITE((uint8_t)0x40);
  uint8_t bytesOut = 1;
  while(count--) {
    if(bytesOut >= WIRE_MAX) {
      wire->endTransmission();
      wire->beginTransmission(i2caddr);
      WIRE_WRITE((uint8_t)0x40);
      bytesOut = 1;
    }
    WIRE_WRITE(*ptr++);
    bytesOut++;
  }
  wire->endTransmission();
  TRANSACTION_END
#if defined(ESP8266)
  yield();
#endif
}


// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white).
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed, rather a
            different pixel mode of the display hardware is used. When
            enabled, drawing SSD1306_BLACK (value 0) pixels will actually draw white,
            SSD1306_WHITE (value 1) will draw black.
*/
void Custom_SSD1306::invertDisplay(boolean i) {
  TRANSACTION_START
  ssd1306_command1(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
  TRANSACTION_END
}

/*!
    @brief  Dim the display.
    @param  dim
            true to enable lower brightness mode, false for full brightness.
    @return None (void).
    @note   This has an immediate effect on the display, no need to call the
            display() function -- buffer contents are not changed.
*/
void Custom_SSD1306::dim(boolean dim, uint8_t customContrast) {
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  TRANSACTION_START
  ssd1306_command1(SSD1306_SETCONTRAST);
  ssd1306_command1(dim ? 0 : customContrast);
  TRANSACTION_END
}
