#include "Memory.h"

#include <Arduino.h>
#include <EEPROM.h>

const unsigned short INIT_FLAG = 0;
const unsigned short CONTACT_NODE = 3;
const unsigned short CONTACT_LIST_FLAG = 18;
const unsigned short CONTACT_COUNT = 20;
const unsigned short CONTACT_LIST = 21;
const unsigned short MESSAGE_LIST_FLAG = 171;
const unsigned short MESSAGE_COUNT = 173;
const unsigned short MESSAGE_LIST = 174;
const unsigned short OFFSET = 434;

Memory::Memory() {
  if (hasSchema()) return;

  // configure schema
  setSchema();

  // clear node info
  saveNodeInformation(Contact());

  // configure contacts and messages lists
  clearContacts();
  clearMessages();
}

Memory::Memory(Contact node) {
  if (hasSchema()) return;

  // configure schema
  setSchema();

  // write node info
  saveNodeInformation(node);

  // configure contacts and messages lists
  clearContacts();
  clearMessages();
}

unsigned char* Memory::getNodeUUID() {
  Contact node;
  EEPROM.get(CONTACT_NODE, node);
  return node.getUUID();
}

char* Memory::getNodeName() {
  Contact node;
  EEPROM.get(CONTACT_NODE, node);
  return node.getName();
}

unsigned short Memory::getNumberContacts() {
  unsigned char count;
  EEPROM.get(CONTACT_COUNT, count);
  return count;
}

unsigned short Memory::getNumberMessages() {
  unsigned char count;
  EEPROM.get(MESSAGE_COUNT, count);
  return count;
}

Contact Memory::getContact(unsigned short index) {
  Contact contact;
  EEPROM.get(CONTACT_LIST + index * sizeof(Contact), contact);
  return contact;
}

Message Memory::getMessage(unsigned short index) {
  Message message;
  EEPROM.get(MESSAGE_LIST + index * sizeof(Message), message);
  return message;
}

bool Memory::saveContact(Contact contact) {
  unsigned short count = getNumberContacts();
  Serial.print("Contacts: ");
  Serial.println(count);
  if (count >= MAX_CONTACTS) return false;

  EEPROM.put(CONTACT_LIST + count * sizeof(Contact), contact);
  EEPROM.put(CONTACT_COUNT, count + 1);
  return true;
}

void Memory::saveMessage(Message message) {
  unsigned short count = getNumberMessages();
  unsigned short offset = getMessagePointerOffset();

  EEPROM.put(MESSAGE_LIST + offset * sizeof(Message), message);

  if (offset >= MAX_MESSAGES) offset = 0;
  if (count < MAX_MESSAGES) EEPROM.put(MESSAGE_COUNT, count + 1);
}

void Memory::saveNodeInformation(Contact contact) {
  EEPROM.put(CONTACT_NODE, contact);
  Serial.println("Node information saved");
  Serial.println(contact.getName());
  Serial.println(getNodeName());
}

void Memory::reset() {
  for (unsigned short i = 0; i <= OFFSET; i++) {
    EEPROM.write(i, 0x0);
  }
}

bool Memory::hasSchema() {
  Serial.println("Checking schema");

  unsigned char init_flag[3];
  EEPROM.get(INIT_FLAG, init_flag);
  if (init_flag[0] != 0xC0 || init_flag[1] != 0xFF || init_flag[2] != 0xEE) return false;

  unsigned char contact_list_flag[2];
  EEPROM.get(CONTACT_LIST_FLAG, contact_list_flag);
  if (contact_list_flag[0] != 0xFA || contact_list_flag[1] != 0xCE) return false;

  unsigned char message_list_flag[2];
  EEPROM.get(MESSAGE_LIST_FLAG, message_list_flag);
  if (message_list_flag[0] != 0xCA || message_list_flag[1] != 0x11) return false;

  return true;
}

void Memory::setSchema() {
  EEPROM.write(INIT_FLAG, 0xC0);
  EEPROM.write(INIT_FLAG + 1, 0xFF);
  EEPROM.write(INIT_FLAG + 2, 0xEE);

  EEPROM.write(CONTACT_LIST_FLAG, 0xFA);
  EEPROM.write(CONTACT_LIST_FLAG + 1, 0xCE);

  EEPROM.write(MESSAGE_LIST_FLAG, 0xCA);
  EEPROM.write(MESSAGE_LIST_FLAG + 1, 0x11);
}

void Memory::clearMessages() {
  Message message;
  for (unsigned char i = 0; i < MAX_MESSAGES; ++i) {
    EEPROM.put(MESSAGE_LIST + i * sizeof(Message), message);
  }
  EEPROM.write(MESSAGE_COUNT, 0);
}

void Memory::clearContacts() {
  Contact contact;
  for (unsigned char i = 0; i < MAX_CONTACTS; ++i) {
    EEPROM.put(CONTACT_LIST + i * sizeof(Contact), contact);
  }
  EEPROM.write(CONTACT_COUNT, 0);
}

unsigned short Memory::getMessagePointerOffset() {
  unsigned char offset;
  EEPROM.get(OFFSET, offset);
  return offset;
}

void Memory::print() {
  for (unsigned short i = 0; i <= OFFSET; ++i) {
    if (i == CONTACT_NODE ||
        i == CONTACT_LIST_FLAG ||
        i == CONTACT_COUNT ||
        i == CONTACT_LIST ||
        i == MESSAGE_LIST_FLAG ||
        i == MESSAGE_COUNT ||
        i == MESSAGE_LIST ||
        i == OFFSET)
      Serial.println();
    Serial.print(EEPROM.read(i), HEX);
    Serial.print(" ");
  }
}
