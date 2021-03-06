#include "Message.h"

#include <Arduino.h>

Message::Message() : from({0, 0, 0, 0, 0}), to({0, 0, 0, 0, 0}), payload(0), length(0) {}

Message::Message(unsigned char* from, unsigned char* to, unsigned short payload, unsigned char length)
    : payload(payload), length(length) {
  setFrom(from);
  setTo(to);
}

Message::Message(unsigned char* from, unsigned char* to, char const* message) {
  setFrom(from);
  setTo(to);
  length = 0;
  payload = 0;
  for (unsigned char i = 0;; ++i) {
    if (message[i] == '\0') {
      length = i;
      break;
    }
    payload |= (message[i] == '-') << i;
  }
}

void Message::setLength(unsigned char length) { this->length = length; }
void Message::setTo(unsigned char* to) { memcpy(this->to, to, 5); }
void Message::setFrom(unsigned char* from) { memcpy(this->from, from, 5); }
void Message::setPayload(unsigned short payload) { this->payload = payload; }

unsigned char Message::getLength() { return length; }
unsigned char* Message::getTo() { return to; }
unsigned char* Message::getFrom() { return from; }
unsigned short Message::getPayload() { return payload; }

char* Message::getPayloadString() {
  return payloadToString(payload, length);
}

unsigned short Message::stringToPayload(char const* message) {
  unsigned short payload = 0;
  for (unsigned char i = 0;; ++i) {
    if (message[i] == '\0')
      break;
    payload |= (message[i] == '-') << i;
  }
  return payload;
}

char* Message::payloadToString(unsigned short payload, unsigned char length) {
  char* payloadString = new char[length + 1];
  for (unsigned char i = 0; i < length; ++i) {
    payloadString[i] = ((payload >> i) & 1) ? '-' : '.';
  }
  payloadString[length] = '\0';
  return payloadString;
}
