#ifndef EEPROM_H_
#define EEPROM_H_
#include <LiquidCrystal.h>
#include <avr/eeprom.h>
#include <limits.h>

class Eeprom {
 public:
  Eeprom() {
  }
  byte read(unsigned int addr) {
    while (EECR & (1 << EEPE))
      ;
    EEARL = addr;  // set the address
    EECR |= (1 << EERE);
    return EEDR;
  }

  void write(unsigned int addr, byte value) {
    while (EECR & (1 << EEPE))
      ;
    EECR &= ~((1 << EEPM1) | (1 << EEPM0));
    EEARL = addr;
    EEDR = value;
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
  }
};

#endif
