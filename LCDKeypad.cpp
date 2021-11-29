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

unsigned long long lastButtonUpdate = 0;
LCDKeypad::Button LCDKeypad::getButtonPress() {
  unsigned long long currentTime = millis();

  // ensure enough time has passed since the last button press
  if (currentTime - lastButtonUpdate > DEBOUNCE_TIME) {
    lastButtonUpdate = currentTime;

    unsigned short analogValue = analogRead(A0);

    // check if each button is pressed and, if so, return the button
    for (Button btn : BUTTONS) {
      unsigned short diff = analogValue - static_cast<unsigned short>(btn);
      if (abs(diff) < MAX_VALUE_ERROR)
        return btn;
    }
  }

  // base case
  return Button::NONE;
}
