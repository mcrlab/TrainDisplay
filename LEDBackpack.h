#ifndef LEDBackpack_h
#define LEDBackpack_h

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

 #include <Wire.h>
 

#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0


// this is the raw HT16K33 controller
class LEDBackpack {
 public:
  LEDBackpack(void);
  void begin(uint8_t _addr);
  void setBrightness(uint8_t b);
  void blinkRate(uint8_t b);
  void writeDisplay(void);
  void clear(void);

  uint16_t displaybuffer[8]; 

  void init(uint8_t a);
 protected:
  uint8_t i2c_addr;
};

class AlphaNum4 : public LEDBackpack {
 public:
  AlphaNum4(void);
  void writeDigitAscii(uint8_t n, uint8_t ascii, boolean dot = false);

 private:


};

#endif // LEDBackpack_h

