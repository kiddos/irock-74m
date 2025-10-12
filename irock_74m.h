#ifndef DEVICE_H
#define DEVICE_H

#include <hidapi/hidapi.h>

namespace irock {

const unsigned short VENDOR_ID = 0x258a;  // change to your device's VID
const unsigned short PRODUCT_ID = 0x0107; // change to your device's PID
const unsigned int INTERFACE = 1;

#define MODE_OFFSET 5
#define BRIGHTNESS_OFFSET 8
#define SPEED_OFFSET 7

enum Mode {
  BREATHING = 0x03,
  STEADY = 0x01,
  PRESS_AND_DESTROY = 0x04,
  NEON_STREAM = 0x05,
  STREAMER = 0x06,
  AMBILIGHT = 0x07,
  DRIPPING_RIPPLE = 0x08,
  BRILLIANT_POINT = 0x09,
  FLASH_AWAY = 0x0A,
  SHADOW_DISAPPEAR = 0x0B,
  RIPPLE_SHINING = 0x0C,
  RICH_ANDD_HONOURED = 0x0D,
  MARQUEE_EFFECT = 0x0E,
  ROTATING_STORM = 0x0F,
  SERPENTINE_HORSE_RACE = 0x10,
  STARS_TWINKLE = 0x11,
  RETRO_SNAKE = 0x12,
  DIAGNAL_TRANSFORMATION = 0x13,
  SINE_WAVE = 0x14,
  OFF = 0x15,
};

enum Brightness {
  BRIGHTNESS_0 = 0,
  BRIGHTNESS_1 = 1,
  BRIGHTNESS_2 = 2,
  BRIGHTNESS_3 = 3,
  BRIGHTNESS_4 = 4,
  BRIGHTNESS_5 = 5,
};

enum Speed {
  SPEED_0 = 0,
  SPEED_1 = 1,
  SPEED_2 = 2,
  SPEED_3 = 3,
  SPEED_4 = 4,
  SPEED_5 = 5,
};

class Keyboard74M {
 public:
  Keyboard74M();

  void Close();

  bool SetConfiguration(Mode mode, Brightness b, Speed s);

 private:
  hid_device *handle_;
};

} // end of irock namespace

#endif /* end of include guard: DEVICE_H */
