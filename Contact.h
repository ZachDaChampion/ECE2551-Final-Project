#ifndef CONTACT_H_
#define CONTACT_H_

#include <iostream>

class Contact {
 public:
  Contact();
  Contact(const Contact& other);
  Contact(unsigned char* givenUUID, char const* givenName);
  Contact(unsigned char* givenUUID, char givenName);
  ~Contact();
  void setUUID(unsigned char* givenUUID);
  void setName(char const* givenName);
  void setName(char givenName);
  unsigned char* getUUID();
  char* getName();

 private:
  unsigned char* uuid;
  char* name;
};

#endif