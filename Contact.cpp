#include "Contact.h"

Contact::Contact() {}

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
  for (i = 0; i < 11; ++i) {
    if (givenName[i] == '\0')
      break;
    name[i] = givenName[i];
  }

  void Contact::setName(char givenName) {
    name[0] = givenName;
    name[1] = '\0';
  }

  unsigned char* Contact::getUUID() {
    return uuid;
  }

  char* Contact::getName() {
    return name;
  }
