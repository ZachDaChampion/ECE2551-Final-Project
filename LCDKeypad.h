#ifndef LCD_KEYPAD_H
#define LCD_KEYPAD_H

#include <LiquidCrystal.h>

class LCDKeypad : public LiquidCrystal {
 public:
  LCDKeypad(uint8_t rs, uint8_t enable,
            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t btn);

  // identifies a button by its approximate analog value
  enum class Button : unsigned short { RIGHT = 0,
                                       UP = 145,
                                       DOWN = 329,
                                       LEFT = 505,
                                       SELECT = 741,
                                       NONE = 1023 };

  // get which button is currently pressed
  Button getButtonPress();

 private:
  unsigned long long lastButtonUpdate;
  uint8_t buttonPin;
};

#endif