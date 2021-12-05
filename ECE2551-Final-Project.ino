#include "Contact.h"
#include "Entropy/Entropy.h"
#include "LCDKeypad.h"
#include "Memory.h"
#include "Message.h"
#include "RF24.h"

#define ENABLE_MEMORY 0
#define ENABLE_RADIO 0

// NR24 radio pins
const uint8_t RADIO_CE_PIN = A1;
const uint8_t RADIO_CSN_PIN = A2;
const uint8_t RADIO_MOSI_PIN = 11;
const uint8_t RADIO_MISO_PIN = 12;
const uint8_t RADIO_SCK_PIN = 13;
const uint8_t RADIO_IRQ_PIN = 2;

// lcd pins
const uint8_t LCD_BUTTONS_PIN = A0;
const uint8_t LCD_DB4_PIN = 4;
const uint8_t LCD_DB5_PIN = 5;
const uint8_t LCD_DB6_PIN = 6;
const uint8_t LCD_DB7_PIN = 7;
const uint8_t LCD_RS_PIN = 8;
const uint8_t LCD_ENABLE_PIN = 9;
const uint8_t LCD_BACKLIGHT_PIN = 10;

// generic state
struct State {
  void (*enter)();
  void (*loop)();
  void (*exit)();
};

// state declarations
extern const State STATE_SETUP;
extern const State STATE_MENU;
extern const State STATE_CONTACTS;
extern const State STATE_MESSAGES;
extern const State STATE_NEW_CONTACT_NAME;
extern const State STATE_NEW_CONTACT_UUID;
extern const State STATE_CONTACT_ADDED;
extern const State STATE_LIST_FULL;
extern const State STATE_ABOUT_ME;
extern const State STATE_NEW_MESSAGE;
extern const State STATE_MESSAGE_SENT;
extern const State STATE_MESSAGE_FAILED;
extern const State STATE_MESSAGE_OPEN;
extern const State STATE_MESSAGE_RECEIVED;

unsigned char contactCount = 0;
Contact contacts[10];

unsigned char messageCount = 0;
Message messages[20];

State state;
unsigned char myUUID[] = {1, 2, 3, 4, 5};
Contact currentContact;
Message currentMessage;
LCDKeypad lcdKeypad(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN, LCD_BUTTONS_PIN);
#if MEMORY_ENABLED
Memory memory;
#endif
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

// menu navigation
unsigned char menuIndex = 0;
char menuChar = 'A';
unsigned char menuCursor = 0;

void setup() {
  // configure radio pins
  pinMode(RADIO_CE_PIN, OUTPUT);
  pinMode(RADIO_CSN_PIN, OUTPUT);
  pinMode(RADIO_MOSI_PIN, OUTPUT);
  pinMode(RADIO_MISO_PIN, INPUT);
  pinMode(RADIO_SCK_PIN, OUTPUT);
  pinMode(RADIO_IRQ_PIN, INPUT);

  // configure lcd pins
  pinMode(LCD_BUTTONS_PIN, INPUT);
  pinMode(LCD_DB4_PIN, OUTPUT);
  pinMode(LCD_DB5_PIN, OUTPUT);
  pinMode(LCD_DB6_PIN, OUTPUT);
  pinMode(LCD_DB7_PIN, OUTPUT);
  pinMode(LCD_RS_PIN, OUTPUT);
  pinMode(LCD_ENABLE_PIN, OUTPUT);
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);

  // configure serial
  Serial.begin(9600);
  while (!Serial)
    ;

// configure memory
#if ENABLE_MEMORY
  myUUID = memory.getNodeUUID();
  contactCount = memory.getNumberContacts();
  messageCount = memory.getNumberMessages();
  for (unsigned char i = 0; i < contactCount; i++) contacts[i] = memory.getContact(i);
  for (unsigned char i = 0; i < messageCount; i++) messages[i] = memory.getMessage(i);
#else
  contactCount = 2;
  unsigned char u1[5] = {1, 2, 3, 4, 5};
  unsigned char u2[5] = {5, 4, 3, 2, 1};
  contacts[0].setName("Zach");
  contacts[0].setUUID(u1);
  contacts[1].setName("Long name");
  contacts[1].setUUID(u2);

  messageCount = 2;
  messages[0].setFrom(u1);
  messages[0].setTo(u2);
  messages[0].setPayload(0b11010);
  messages[0].setLength(5);

  messages[1].setFrom(u2);
  messages[1].setTo(u1);
  messages[1].setPayload(0b10);
  messages[1].setLength(5);
#endif

// configure radio
#if ENABLE_RADIO
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
      ;
  }
#endif

  // configure keypad
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
  lcdKeypad.begin(16, 2);
  lcdKeypad.clear();

  state = STATE_SETUP;
  state.enter();

  Serial.println(memcmp(u1, u1, 5));
  Serial.println(memcmp(u1, u2, 5));
  Serial.println(memcmp(myUUID, u1, 5));
  Serial.println(memcmp(myUUID, contacts[0].getUUID(), 5));
  Serial.println(getContactFromUUID(messages[0].getFrom()));

  for (unsigned char i = 0; i < 5; ++i) Serial.print(messages[0].getFrom()[i]);
  Serial.println();
  for (unsigned char i = 0; i < 5; ++i) Serial.print(messages[1].getFrom()[i]);
  Serial.println();

  Serial.println("all set up");
}

void loop() {
  if (state.loop)
    state.loop();
}

// transition between states
void stateTransition(State newState) {
  if (state.exit)
    state.exit();
  state = newState;
  if (state.enter)
    state.enter();
}

// get contact from uuid
char* getContactFromUUID(unsigned char uuid[]) {
  for (unsigned char i = 0; i < contactCount; i++) {
    if (memcmp(contacts[i].getUUID(), uuid, 5) == 0)
      return contacts[i].getName();
  }
  return "";
}

// setup state
const State STATE_SETUP = {
    .enter = []() {
      Serial.println("MENU");
// check schema and, if it passes, go to menu state
#if ENABLE_MEMORY
      if (memory.hasSchema()) {
        stateTransition(STATE_MENU);
        return;
      }
#else
      stateTransition(STATE_MENU);
      return;
#endif

      // print welcome message
      lcdKeypad.clear();
      lcdKeypad.print("Welcome!");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("Name: ");
    },
    .loop = nullptr,  // TODO: enter name
    .exit = nullptr,
};

// menu state
const char MENU_ITEMS[][17] = {
    "<-  Contacts  ->",
    "<-  Messages  ->",
    "<- N. Contact ->",
    "<-  About Me  ->",
};
const State STATE_MENU = {

    .enter = []() {
      Serial.println("MENU");
      lcdKeypad.clear();
      lcdKeypad.print("Menu:");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print(MENU_ITEMS[0]);
      menuIndex = 0; },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();

      // check for button press
      switch (button) {

        // navigate left
        case LCDKeypad::Button::LEFT:
          if (menuIndex == 0)
            menuIndex = 3;
          else
            --menuIndex;
          lcdKeypad.setCursor(0, 1);
          lcdKeypad.print(MENU_ITEMS[menuIndex]);
          break;

        // navigate right
        case LCDKeypad::Button::RIGHT:
          if (menuIndex == 3)
            menuIndex = 0;
          else
            ++menuIndex;
          lcdKeypad.setCursor(0, 1);
          lcdKeypad.print(MENU_ITEMS[menuIndex]);
          break;

        // select menu item
        case LCDKeypad::Button::SELECT:
          switch (menuIndex) {
            case 0:
             stateTransition(STATE_CONTACTS);
              return;
            case 1:
             stateTransition(STATE_MESSAGES);
              return;
            case 2:
//              stateTransition(STATE_NEW_CONTACT);
              return;
            case 3:
//              stateTransition(STATE_ABOUT_ME);
              return;
          }
          break;

          default: break;
    } },

    .exit = nullptr,
};

const State STATE_CONTACTS = {
    .enter = []() {
      Serial.println("CONTACTS");
      lcdKeypad.clear();
      lcdKeypad.print("Contact:");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print(contactCount ? contacts[0].getName() : "No contacts");
      menuIndex = 0; },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();

      // check for button press
      switch (button) {

        // navigate left
        case LCDKeypad::Button::LEFT:
          if (menuIndex == 0)
            menuIndex = contactCount - 1;
          else
            --menuIndex;
          lcdKeypad.clearLine(1);
          lcdKeypad.print(contactCount ? contacts[menuIndex].getName() : "No contacts");
          break;

        // navigate right
        case LCDKeypad::Button::RIGHT:
          if (menuIndex == contactCount - 1)
            menuIndex = 0;
          else
            ++menuIndex;
          lcdKeypad.clearLine(1);
          lcdKeypad.print(contactCount ? contacts[menuIndex].getName() : "No contacts");
          break;

        // go back
        case LCDKeypad::Button::UP:
          stateTransition(STATE_MENU);
          return;

        // select menu item
        case LCDKeypad::Button::SELECT:
          currentContact = contacts[menuIndex];
          // stateTransition(STATE_NEW_MESSAGE);
          return;

          default: break;
      } },

    .exit = nullptr,
};

const State STATE_MESSAGES = {
    .enter = []() {
      Serial.println("MESSAGES");
      lcdKeypad.clear();
      bool from = 0;
      if (messageCount) {
        from = memcmp(myUUID, messages[menuIndex].getFrom(), 5) == 0;
        lcdKeypad.print("Message:      [");
        lcdKeypad.print(from ? 'S' : 'R');
      }
      else lcdKeypad.print("Message:");
      lcdKeypad.setCursor(0, 1);
      if (messageCount) {
        lcdKeypad.print("1. ");
                lcdKeypad.print(from ? getContactFromUUID(messages[menuIndex].getTo()) : getContactFromUUID(messages[menuIndex].getFrom()));
      }
      else lcdKeypad.print("No messages");

      menuIndex = 0; },

    .loop = []() {
        LCDKeypad::Button button = lcdKeypad.getButtonPress();

        // check for button press
        switch (button) {

          // navigate left
          case LCDKeypad::Button::LEFT: {
            if (!messageCount) break;
            if (menuIndex == 0)
              menuIndex = messageCount - 1;
            else
              --menuIndex;
            bool from = memcmp(myUUID, messages[menuIndex].getFrom(), 5) == 0;
            lcdKeypad.setCursor(15, 0);
            lcdKeypad.print(from ? 'S' : 'R');
            lcdKeypad.clearLine(1);
            lcdKeypad.print(menuIndex + 1);
            lcdKeypad.print(". ");
            lcdKeypad.print(from ? getContactFromUUID(messages[menuIndex].getTo()) : getContactFromUUID(messages[menuIndex].getFrom()));
          } break;

          // navigate right
          case LCDKeypad::Button::RIGHT: {
            if (!messageCount) break;
            if (menuIndex == messageCount - 1)
              menuIndex = 0;
            else
              ++menuIndex;
            bool from = memcmp(myUUID, messages[menuIndex].getFrom(), 5) == 0;
            lcdKeypad.setCursor(15, 0);
            lcdKeypad.print(memcmp(myUUID, messages[menuIndex].getFrom(), 5) == 0 ? 'S' : 'R');
            lcdKeypad.clearLine(1);
            lcdKeypad.print(menuIndex + 1);
            lcdKeypad.print(". ");
            lcdKeypad.print(from ? getContactFromUUID(messages[menuIndex].getTo()) : getContactFromUUID(messages[menuIndex].getFrom()));
          } break;

          // go back
          case LCDKeypad::Button::UP:
            stateTransition(STATE_MENU);
            return;

          // select menu item
          case LCDKeypad::Button::SELECT:
            currentMessage = messages[menuIndex];
//            stateTransition(STATE_NEW_MESSAGE);
            return;

          default: break;
        } },

    .exit = nullptr,
};

const State STATE_NEW_CONTACT_NAME = {
    .enter = []() {},

    .loop = []() {},

    .exit = nullptr,
};