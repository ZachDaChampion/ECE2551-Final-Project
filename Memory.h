#ifndef MEMORY_H_
#define MEMORY_H_
#include "Eeprom.h"

class Memory {
  public:
    Memory();
    Memory(Contact node);
    unsigned char* getNodeUUID();
    char* getNodeName();
    unsigned short getNumberContacts();
    unsigned short getNumberMessages();
    Contact getContact(unsigned short index);
    Message getMessage(unsigned short index);
    bool saveContact(Contact contact);
    void saveMessage(Message message);
    void saveNodeInformation(Contact contact);
    // Add as you see fit

  protected:
    bool hasSchema();
    void setSchema();
    void clearMessages();
    void clearContacts();
    unsigned short getMessagePointerOffset();
    // Add as you see fit

  private:
    const unsigned short MAX_CONTACTS = 10;
    const unsigned short MAX_MESSAGES = 20;
    Eeprom x;
    // Add as you see fit

};




#endif
