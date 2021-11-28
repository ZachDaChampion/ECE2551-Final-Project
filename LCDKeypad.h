#ifndef LCD_KEYPAD_H
#define LCD_KEYPAD_H

#include <LiquidCrystal.h>

class LCDKeypad : public LiquidCrystal {
 public:
  // identifies a button by its approximate analog value
  enum class Button : unsigned short { RIGHT = 0,
                                       UP = 145,
                                       DOWN = 329,
                                       LEFT = 505,
                                       SELECT = 741,
                                       NONE = 1023 };

  // get which button is currently pressed
  Button getButtonPress();
};

#endif