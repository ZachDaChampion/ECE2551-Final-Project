#include "Contact.h"

#include <Arduino.h>

Contact::Contact() : uuid({0, 0, 0, 0, 0}), name("") {}

Contact::Contact(unsigned char* givenUUID, char const* givenName) {
  setUUID(givenUUID);
  setName(givenName);
}

Contact::Contact(unsigned char* givenUUID, char givenName) {
  setUUID(givenUUID);
  setName(givenName);
}

void Contact::setUUID(unsigned char* givenUUID) {
  memcpy(uuid, givenUUID, 5);
}

void Contact::setName(char const* givenName) {
  unsigned char i;
  for (i = 0; i < 10; ++i) {
    name[i] = givenName[i];
    if (givenName[i] == '\0')
      break;
  }
}

void Contact::setName(char givenName) {
  name[0] = givenName;
  name[1] = '\0';
}

unsigned char* Contact::getUUID() {
  return uuid;
}

char* Contact::getName() {
  static char n[11];
  unsigned char i;
  for (i = 0; i < 10; ++i) {
    if (name[i] == '\0') {
      break;
    }
    n[i] = name[i];
  }
  n[i] = '\0';
  return n;
}
