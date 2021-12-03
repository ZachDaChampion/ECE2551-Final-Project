#include "Contact.h"

Contact::Contact() {
  uuid = new unsigned char[5]();
  name = new char[10]();
}

Contact::Contact(const Contact& other) {
  uuid = new unsigned char[5]();
  name = new char[10]();
  for (unsigned char i = 0; i < 5; ++i)
    uuid[i] = other.uuid[i];
  for (unsigned char i = 0; i < 10; ++i)
    name[i] = other.name[i];
}

Contact::Contact(unsigned char* givenUUID, char const* givenName) {
  uuid = new unsigned char[5]();
  name = new char[10]();
  setUUID(givenUUID);
  setName(givenName);
}

Contact::Contact(unsigned char* givenUUID, char givenName) {
  uuid = new unsigned char[5]();
  name = new char[10]();
  setUUID(givenUUID);
  setName(givenName);
}

Contact::~Contact() {
  delete[] uuid;
  delete[] name;
}

void Contact::setUUID(unsigned char* givenUUID) {
  for (unsigned char i = 0; i < 5; ++i)
    uuid[i] = givenUUID[i];
}

void Contact::setName(char const* givenName) {
  unsigned char i;
  for (i = 0; i < 10; ++i) {
    if (givenName[i] == '\0')
      break;
    name[i] = givenName[i];
  }
  for (; i < 10; ++i) name[i] = ' ';
}

void Contact::setName(char givenName) {
  name[0] = givenName;
  for (char i = 1; i < 10; ++i)
    name[i] = ' ';
}

unsigned char* Contact::getUUID() {
  return uuid;
}

char* Contact::getName() {
  return name;
}
