#define ENABLE_MEMORY 1
#define ENABLE_RADIO 1

#ifdef ENABLE_RADIO
#include <RF24.h>
#endif

#include <Entropy.h>

#include "Contact.h"
#include "LCDKeypad.h"
#include "Message.h"

#ifdef ENABLE_MEMORY
#include "Memory.h"
#endif

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

// buzzer pin
const uint8_t BUZZER_PIN = 3;

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
State prevState;
Contact myContact;
Contact currentContact;
Message currentMessage;
Message incomingMessage;
LCDKeypad lcdKeypad(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN, LCD_BUTTONS_PIN);
#if ENABLE_MEMORY
Memory memory;
#endif
RF24 radio(RADIO_CE_PIN, RADIO_CSN_PIN);

// menu navigation
unsigned char menuIndex = 0;
char menuChar = 'A';
char menuCharUUID = 0x0;
unsigned char menuCursor = 0;
char menuInput[17] = {};
unsigned char menuInputUUID[17] = {};
unsigned long long stateTime = 0;

void setup() {
  // configure radio pins
  pinMode(RADIO_IRQ_PIN, INPUT_PULLUP);

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
  // memory.reset();
  updateMemory();
#else
  contactCount = 3;
  unsigned char u1[5] = {1, 2, 3, 4, 5};
  unsigned char u2[5] = {5, 4, 3, 2, 1};
  contacts[0].setName("Zach");
  contacts[0].setUUID(u1);
  contacts[1].setName("Long name!!!");
  contacts[1].setUUID(u2);

  contacts[2] = Contact(u2, "Other");

  messageCount = 2;
  messages[0].setFrom(u1);
  messages[0].setTo(u2);
  messages[0].setPayload(0b11010);
  messages[0].setLength(5);

  messages[1].setFrom(u2);
  messages[1].setTo(u1);
  messages[1].setPayload(0b10);
  messages[1].setLength(5);

  myContact.setUUID(u1);
  myContact.setName("Zach");
#endif

// configure radio
#if ENABLE_RADIO
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1)
      ;
  }
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(Message));
  radio.setAddressWidth(5);
  radio.openReadingPipe(1, myContact.getUUID());
  radio.startListening();
  attachInterrupt(digitalPinToInterrupt(RADIO_IRQ_PIN), radioInterruptHandler, FALLING);
#endif

  // configure keypad
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
  lcdKeypad.begin(16, 2);
  lcdKeypad.clear();

  state = STATE_SETUP;
  state.enter();
}

void loop() {
#if ENABLE_MEMORY
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') {
      Serial.println(F("reset"));
      memory.reset();
    }
    if (c == 'm') memory.print();
  }
#endif
#if ENABLE_RADIO
  if (!radio.isChipConnected())
    Serial.println(F("radio is not connected!!"));
#endif
  if (state.loop)
    state.loop();
}

#if ENABLE_MEMORY
void updateMemory() {
  myContact.setUUID(memory.getNodeUUID());
  myContact.setName(memory.getNodeName());
  contactCount = memory.getNumberContacts();
  messageCount = memory.getNumberMessages();
  for (unsigned char i = 0; i < contactCount; i++) contacts[i] = memory.getContact(i);
  for (unsigned char i = 0; i < messageCount; i++) messages[i] = memory.getMessage(i);
}
#endif

#if ENABLE_RADIO
void radioInterruptHandler() {
  if (radio.available()) {
    radio.read(&incomingMessage, sizeof(Message));

#if ENABLE_MEMORY
    Serial.println(F("received message"));
    Serial.println(incomingMessage.getPayload(), BIN);
    for (unsigned char i = 0; i < 5; i++)
      Serial.print(incomingMessage.getFrom()[i], HEX);
    Serial.println();
    memory.saveMessage(incomingMessage);
    updateMemory();
#else
    if (messageCount == 20)
      messages[0] = incomingMessage;
    else {
      messages[messageCount] = incomingMessage;
      ++messageCount;
    }
#endif

    tone(BUZZER_PIN, 1000, 100);

    stateTransition(STATE_MESSAGE_RECEIVED);
  }
}
#endif

// transition between states
bool saveState = true;
void stateTransition(State newState) {
  if (state.exit)
    state.exit();
  if (saveState)
    prevState = state;
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

// string input handler
void loopTextInput(LCDKeypad::Button button, unsigned char offset, unsigned char maxLength) {
  // check for button press
  switch (button) {
    // navigate left
    case LCDKeypad::Button::LEFT:
      if (menuCursor == 0)
        break;
      lcdKeypad.write(' ');
      --menuCursor;
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    // navigate right
    case LCDKeypad::Button::RIGHT:
      if (menuCursor == maxLength - 1)
        break;
      ++menuCursor;
      menuInput[menuCursor] = 'A';
      menuChar = 'A';
      lcdKeypad.setCursor(menuCursor + offset, 1);
      lcdKeypad.write(menuChar);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    // next character
    case LCDKeypad::Button::DOWN:
      if (menuChar == 'Z')
        menuChar = 'a';
      else if (menuChar == 'z')
        menuChar = 'A';
      else
        ++menuChar;
      menuInput[menuCursor] = menuChar;
      lcdKeypad.write(menuChar);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

      // prev character
    case LCDKeypad::Button::UP:
      if (menuChar == 'A')
        menuChar = 'z';
      else if (menuChar == 'a')
        menuChar = 'Z';
      else
        --menuChar;
      menuInput[menuCursor] = menuChar;
      lcdKeypad.write(menuChar);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    default:
      break;
  }
}

// uuid input handler
void loopUUIDInput(LCDKeypad::Button button, unsigned char offset, unsigned char maxLength) {
  // check for button press
  switch (button) {
    // navigate left
    case LCDKeypad::Button::LEFT:
      if (menuCursor == 0)
        break;
      lcdKeypad.print(0x0, HEX);
      menuInputUUID[menuCursor] = 0x0;
      --menuCursor;
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    // navigate right
    case LCDKeypad::Button::RIGHT:
      if (menuCursor == maxLength - 1)
        break;
      ++menuCursor;
      menuInputUUID[menuCursor] = 0x0;
      menuCharUUID = 0x0;
      lcdKeypad.setCursor(menuCursor + offset, 1);
      lcdKeypad.print(menuCharUUID, HEX);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    // next character
    case LCDKeypad::Button::DOWN:
      if (menuCharUUID == 0x0)
        menuCharUUID = 0xF;
      else
        --menuCharUUID;
      menuInputUUID[menuCursor] = menuCharUUID;
      lcdKeypad.print(menuCharUUID, HEX);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

      // prev character
    case LCDKeypad::Button::UP:
      if (menuCharUUID == 0xF)
        menuCharUUID = 0x0;
      else
        ++menuCharUUID;
      menuInputUUID[menuCursor] = menuCharUUID;
      lcdKeypad.print(menuCharUUID, HEX);
      lcdKeypad.setCursor(menuCursor + offset, 1);
      break;

    default:
      break;
  }
}

// setup state
const State STATE_SETUP = {
    .enter = []() {
// check schema and, if it passes, go to menu state
#if ENABLE_MEMORY
      for (unsigned char i = 0; i < 5; ++i) {
        unsigned char* uuid = memory.getNodeUUID();
        if (uuid[i] != 0) {
          stateTransition(STATE_MENU);
          return;
        }
      }
#else
// stateTransition(STATE_MENU);
// return;
#endif

      // print welcome message
      menuIndex = 0;
      menuChar = 'A';
      menuCursor = 0;
      menuInput[0] = 'A';
      lcdKeypad.clear();
      lcdKeypad.print("Welcome!");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("Name: A");
      lcdKeypad.setCursor(menuCursor + 6, 1);
      lcdKeypad.cursor(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (button == LCDKeypad::Button::SELECT) {
        unsigned char uuid[5];
        Entropy.initialize();
        for (unsigned char i = 0; i < 5; ++i)
          uuid[i] = Entropy.random(0xFF);
        myContact = Contact(uuid, menuInput);
#if ENABLE_MEMORY
        memory.saveNodeInformation(myContact);
#endif
        stateTransition(STATE_MENU);
        return;
      }
      
      // handle text input
      loopTextInput(button, 6, 10); },

    .exit = []() { lcdKeypad.noCursor(); },
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
             menuIndex = 0;
             stateTransition(STATE_CONTACTS);
              return;
            case 1:
             menuIndex = 0;
             stateTransition(STATE_MESSAGES);
              return;
            case 2:
             stateTransition(STATE_NEW_CONTACT_NAME);
              return;
            case 3:
             stateTransition(STATE_ABOUT_ME);
              return;
          }
          break;

          default: break;
    } },

    .exit = nullptr,
};

const State STATE_CONTACTS = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Contact:");
      lcdKeypad.setCursor(0, 1);
      if (contactCount) {
        lcdKeypad.print("<-            ->");
          lcdKeypad.setCursor(3 + (10 - strlen(contacts[menuIndex].getName())) / 2, 1);
          lcdKeypad.print(contacts[menuIndex].getName());
      }
      else lcdKeypad.print("  No Contacts   "); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();

      // check for button press
      switch (button) {

        // navigate left
        case LCDKeypad::Button::LEFT:
          if (!contactCount) break;
          if (menuIndex == 0)
            menuIndex = contactCount - 1;
          else
            --menuIndex;
          lcdKeypad.setCursor(3, 1);
          lcdKeypad.print("          ");
          lcdKeypad.setCursor(3 + (10 - strlen(contacts[menuIndex].getName())) / 2, 1);
          Serial.print("Contact: ");
          Serial.println(contacts[menuIndex].getName());
          Serial.print("Size: ");
          Serial.println(strlen(contacts[menuIndex].getName()));
          for (unsigned char i = 0; i < 11; ++i) {
            Serial.print(contacts[menuIndex].getName()[i], HEX);
            Serial.print(' ');
          }
          Serial.println();
          lcdKeypad.print(contacts[menuIndex].getName());
          break;

        // navigate right
        case LCDKeypad::Button::RIGHT:
          if (!contactCount) break;
          if (menuIndex == contactCount - 1)
            menuIndex = 0;
          else
            ++menuIndex;
          lcdKeypad.setCursor(3, 1);
          lcdKeypad.print("          ");
          lcdKeypad.setCursor(3 + (10 - strlen(contacts[menuIndex].getName())) / 2, 1);
          lcdKeypad.print(contacts[menuIndex].getName());
          break;

        // go back
        case LCDKeypad::Button::UP:
          stateTransition(STATE_MENU);
          return;

        // select menu item
        case LCDKeypad::Button::SELECT:
        if (contactCount) {
          currentContact.setName(contacts[menuIndex].getName());
          currentContact.setUUID(contacts[menuIndex].getUUID());
          stateTransition(STATE_NEW_MESSAGE);
          return;
        }
        break;

          default: break;
      } },

    .exit = nullptr,
};

void showCurrentMessagePreview() {
  bool from = memcmp(myContact.getUUID(), messages[menuIndex].getFrom(), 5) == 0;
  lcdKeypad.setCursor(15, 0);
  lcdKeypad.print(from ? 'S' : 'R');
  lcdKeypad.clearLine(1);
  lcdKeypad.print(menuIndex + 1);
  lcdKeypad.print(". ");
  lcdKeypad.print(from ? getContactFromUUID(messages[menuIndex].getTo()) : getContactFromUUID(messages[menuIndex].getFrom()));
}
const State STATE_MESSAGES = {
    .enter = []() {
      bool from = 0;
      lcdKeypad.clear();
      if (messageCount) {
        from = memcmp(myContact.getUUID(), messages[menuIndex].getFrom(), 5) == 0;
        lcdKeypad.print("Message:      [");
        lcdKeypad.print(from ? 'S' : 'R');
      } else
        lcdKeypad.print("Message:");
      lcdKeypad.setCursor(0, 1);
      if (messageCount) {
        lcdKeypad.print(menuIndex + 1);
        lcdKeypad.print(". ");
        lcdKeypad.print(from ? getContactFromUUID(messages[menuIndex].getTo()) : getContactFromUUID(messages[menuIndex].getFrom()));
      } else
        lcdKeypad.print("No messages"); },

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
              showCurrentMessagePreview();
          } break;

          // navigate right
          case LCDKeypad::Button::RIGHT: {
            if (!messageCount) break;
            if (menuIndex == messageCount - 1)
              menuIndex = 0;
            else
              ++menuIndex;
              showCurrentMessagePreview();
          } break;

          // go back
          case LCDKeypad::Button::UP:
            stateTransition(STATE_MENU);
            return;

          // select menu item
          case LCDKeypad::Button::SELECT:
            currentMessage = messages[menuIndex];
           stateTransition(STATE_MESSAGE_OPEN);
            return;

          default: break;
        } },

    .exit = nullptr,
};

const State STATE_NEW_CONTACT_NAME = {
    .enter = []() {
      if (contactCount == 10) {
        stateTransition(STATE_LIST_FULL);
        return;
      }
      for (unsigned char i = 0; i < 16; ++i)
        menuInput[i] = '\0';
      menuChar = 'A';
      menuCursor = 0;
      menuInput[0] = 'A';
      lcdKeypad.clear();
      lcdKeypad.print("New Contact");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("Name: A");
      lcdKeypad.setCursor(menuCursor + 6, 1);
      lcdKeypad.cursor(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (button == LCDKeypad::Button::SELECT) {
        menuInput[menuCursor + 1] = '\0';
        currentContact.setName(menuInput);
        stateTransition(STATE_NEW_CONTACT_UUID);
        return;
      }
      loopTextInput(button, 6, 10); },

    .exit = []() { lcdKeypad.noCursor(); },
};

const State STATE_NEW_CONTACT_UUID = {
    .enter = []() {
    menuCharUUID = 0x0;
    menuCursor = 0;
      for (unsigned char i = 0; i < 10; ++i)
        menuInputUUID[i] = 0x0;
    lcdKeypad.clear();
    lcdKeypad.print("New Contact");
    lcdKeypad.setCursor(0, 1);
    lcdKeypad.print("UUID: 0000000000");
    lcdKeypad.setCursor(menuCursor + 6, 1);
    lcdKeypad.cursor(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (button == LCDKeypad::Button::SELECT) {
        unsigned char uuid[5];
        for (int i = 0; i < 5; ++i) {
          uuid[i] = menuInputUUID[2*i + 1];
          uuid[i] += menuInputUUID[2*i] << 4;
        }
        currentContact.setUUID(uuid);

#if ENABLE_MEMORY
        memory.saveContact(currentContact);
        updateMemory();
#else
        contacts[contactCount].setName(currentContact.getName());
        contacts[contactCount].setUUID(currentContact.getUUID());
        ++contactCount;
#endif
        stateTransition(STATE_CONTACT_ADDED);
        return;
      }
      loopUUIDInput(button, 6, 10); },

    .exit = []() { lcdKeypad.noCursor(); },
};

const State STATE_CONTACT_ADDED = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Contact added!");
      stateTime = millis(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (millis() - stateTime > 3000 || button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MENU);
        return;
      } },
    .exit = nullptr,
};

const State STATE_LIST_FULL = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Contact list");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("is full!");
      stateTime = millis(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (millis() - stateTime > 3000 || button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MENU);
        return;
      } },
    .exit = nullptr,
};

const State STATE_ABOUT_ME = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Name: ");
      lcdKeypad.print(myContact.getName());
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("UUID: ");
      for (unsigned char i = 0; i < 5; ++i) {
        lcdKeypad.print(myContact.getUUID()[i] >> 4, HEX);
        lcdKeypad.print(myContact.getUUID()[i] & 0xF, HEX);
      } },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MENU);
        return;
      } },

    .exit = nullptr,
};

const State STATE_NEW_MESSAGE = {

    .enter = []() {
      for (unsigned char i = 0; i < 16; ++i)
        menuInput[i] = '\0';
      menuCursor = 0;
      lcdKeypad.clear();
      lcdKeypad.print("To: ");
      lcdKeypad.print(currentContact.getName());
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.cursor(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();

      switch (button) {

        // go back
        case LCDKeypad::Button::UP:
          stateTransition(STATE_CONTACTS);
          return;

        // add dot
        case LCDKeypad::Button::LEFT:
          menuInput[menuCursor] = '.';
          lcdKeypad.write('.');
          if (menuCursor == 15) lcdKeypad.setCursor(15,1);
          else ++menuCursor;
          break;

          // add dash
        case LCDKeypad::Button::RIGHT:
          menuInput[menuCursor] = '-';
          lcdKeypad.write('-');
          if (menuCursor == 15) lcdKeypad.setCursor(15,1);
          else ++menuCursor;
          break;

          // backspace 
        case LCDKeypad::Button::DOWN:
          if (menuCursor == 0) break;
          menuInput[menuCursor] = '\0';
          --menuCursor;
          lcdKeypad.setCursor(menuCursor, 1);
          lcdKeypad.print(" ");
          lcdKeypad.setCursor(menuCursor, 1);
          break;

          // select
        case LCDKeypad::Button::SELECT:{
          currentMessage = Message(myContact.getUUID(), currentContact.getUUID(), menuInput);

          for (unsigned char i = 0; i < 5; ++i) {
            Serial.print(currentMessage.getTo()[i], HEX);
            Serial.print(" ");
          }
          Serial.println();

// send message
#if ENABLE_RADIO
          radio.stopListening();
          radio.openWritingPipe(currentMessage.getTo());
          bool report = radio.write(&currentContact, sizeof(currentContact));
          if (report)
            stateTransition(STATE_MESSAGE_SENT);
          else
            stateTransition(STATE_MESSAGE_FAILED);
          radio.startListening();
          return;
#else
          
            stateTransition(STATE_MESSAGE_SENT);
            return;
#endif
        } break;

        default: break;
      } },

    .exit = []() { lcdKeypad.noCursor(); },
};

const State STATE_MESSAGE_SENT = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Message Sent!");
      tone(BUZZER_PIN, 1000, 100);
#if ENABLE_MEMORY
      memory.saveMessage(currentMessage);
      updateMemory();
#else
      if (messageCount == 20) messages[0] = currentMessage;
      else {      
        messages[messageCount] = currentMessage;
        ++messageCount; 
      }
#endif
      stateTime = millis(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (millis() - stateTime > 3000 || button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MENU);
        return;
      } },

    .exit = nullptr,
};

const State STATE_MESSAGE_FAILED = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("Message Failed!");
      tone(BUZZER_PIN, 1000, 100);
      stateTime = millis(); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (millis() - stateTime > 3000 || button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MENU);
        return;
      } },

    .exit = nullptr,
};

const State STATE_MESSAGE_OPEN = {
    .enter = []() {
      bool from = memcmp(currentMessage.getFrom(), myContact.getUUID(), 5) == 0;
    lcdKeypad.clear();
    lcdKeypad.print(from ? "Sent: " : "From: ");
    lcdKeypad.print(from ? getContactFromUUID(currentMessage.getTo()) : getContactFromUUID(currentMessage.getFrom()));
    lcdKeypad.setCursor(0, 1);
    lcdKeypad.print(currentMessage.getPayloadString()); },

    .loop = []() {
      LCDKeypad::Button button = lcdKeypad.getButtonPress();
      if (button == LCDKeypad::Button::UP) {
        stateTransition(STATE_MESSAGES);
        return;
      } },

    .exit = nullptr,
};

const State STATE_MESSAGE_RECEIVED = {
    .enter = []() {
      lcdKeypad.clear();
      lcdKeypad.print("New Message!");
      lcdKeypad.setCursor(0, 1);
      lcdKeypad.print("From: ");
      saveState = false;
      lcdKeypad.print(getContactFromUUID(incomingMessage.getFrom()));
      tone(BUZZER_PIN, 1000, 100);
      stateTime = millis(); },

    .loop = []() {
        LCDKeypad::Button button = lcdKeypad.getButtonPress();
        if (millis() - stateTime > 6000 || button == LCDKeypad::Button::UP) {
          stateTransition(prevState);
          return;
        } },

    .exit = []() { saveState = true; },
};
