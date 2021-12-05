#include "Contact.h"

#include <Arduino.h>

Contact::Contact() {
  uuid = new unsigned char[5];
  name = new char[11];
}

Contact::Contact(const Contact& other)
    : uuid(new unsigned char[5]), name(new char[10]) {
  memcpy(uuid, other.uuid, 5);
  memcpy(name, other.name, 10);
}

Contact::Contact(unsigned char* givenUUID, char const* givenName)
    : uuid(new unsigned char[5]), name(new char[11]) {
  setUUID(givenUUID);
  setName(givenName);
}

Contact::Contact(unsigned char* givenUUID, char givenName)
    : uuid(new unsigned char[5]),
      name(new char[11]) {
  setUUID(givenUUID);
  setName(givenName);
}

Contact::~Contact() {
  delete[] uuid;
  delete[] name;
}

void Contact::setUUID(unsigned char* givenUUID) {
  memcpy(uuid, givenUUID, 5);
}

void Contact::setName(char const* givenName) {
  unsigned char i;
  for (i = 0; i < 10; ++i) {
    if (givenName[i] == '\0')
      break;
    name[i] = givenName[i];
  }
  name[i] = '\0';
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
