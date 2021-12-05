#include "LCDKeypad.h"

#include <Arduino.h>

// the time to wait between key presses
unsigned const long long DEBOUNCE_TIME = 50;

// defines the maximum error in the analog input for a button to still be considered pressed
unsigned const short MAX_VALUE_ERROR = 50;

// array of all the buttons
LCDKeypad::Button BUTTONS[6] = {LCDKeypad::Button::UP,
                                LCDKeypad::Button::DOWN,
                                LCDKeypad::Button::LEFT,
                                LCDKeypad::Button::RIGHT,
                                LCDKeypad::Button::SELECT};

LCDKeypad::LCDKeypad(uint8_t rs, uint8_t enable,
                     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t btn)
    : LiquidCrystal(rs, enable, d4, d5, d6, d7), lastButtonUpdate(0), buttonPin(btn) {}

LCDKeypad::Button LCDKeypad::getButtonPress() {
  unsigned long long currentTime = millis();
  unsigned short analogValue = analogRead(buttonPin);

  // check if each button is pressed and, if so, return the button
  for (Button btn : BUTTONS) {
    unsigned short diff = analogValue - static_cast<unsigned short>(btn);
    if (abs(diff) < MAX_VALUE_ERROR) {
      // only return button press if debounce time has passed
      if (currentTime - lastButtonUpdate > DEBOUNCE_TIME) {
        lastButtonUpdate = currentTime;
        return btn;
      }
      lastButtonUpdate = currentTime;
    }
  }

  // base case
  return Button::NONE;
}
