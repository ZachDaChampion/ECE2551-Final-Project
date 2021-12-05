#ifndef LCD_KEYPAD_H
#define LCD_KEYPAD_H

#include <LiquidCrystal.h>

class LCDKeypad : public LiquidCrystal {
 public:
  LCDKeypad(uint8_t rs, uint8_t enable,
            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t btn);

  // identifies a button by its approximate analog value
  enum class Button : unsigned short { RIGHT = 0,
                                       UP = 100,
                                       DOWN = 257,
                                       LEFT = 410,
                                       SELECT = 640,
                                       NONE = 1023 };

  // get which button is currently pressed
  Button getButtonPress();

  void clearLine(unsigned char line);

 private:
  unsigned long long lastButtonUpdate;
  uint8_t buttonPin;
};

#endif
