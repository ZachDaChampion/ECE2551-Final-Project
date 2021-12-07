#ifndef CONTACT_H_
#define CONTACT_H_

class Contact {
 public:
  Contact();
  Contact(unsigned char* givenUUID, char const* givenName);
  Contact(unsigned char* givenUUID, char givenName);
  void setUUID(unsigned char* givenUUID);
  void setName(char const* givenName);
  void setName(char givenName);
  unsigned char* getUUID();
  char* getName();

 private:
  unsigned char uuid[5];
  char name[10];
};

#endif
