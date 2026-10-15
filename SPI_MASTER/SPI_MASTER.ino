#include <SPI.h>    // SPI

// ---------- SPI setup ----------
const int ss1 = 10;  // Slave 1 CS
const int ss2 = 9;   // Slave 2 CS

const int buttonPins[8] = {4, 5, 6, 7, A0, A1, A2, A3};
// Changed last two to A0, A1 since D9, D10 are used for SPI CS lines

void setup() {
  // SPI
  SPI.begin();
  pinMode(ss1, OUTPUT);
  pinMode(ss2, OUTPUT);
  digitalWrite(ss1, HIGH);
  digitalWrite(ss2, HIGH);
  // Buttons
  for (int i = 0; i < 8; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  for (int i = 0; i < 8; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      handleButton(i + 1); // Button numbers 1–8
      delay(200); // debounce
    }
  }
}

void handleButton(int btn) {
  // Decide which slave and what to send
  // diri na di mga btn control e customized lag ang code
  if (btn >= 1 && btn <= 4) {
    // Slave 1 commands
    if (btn == 1) {
      sendToSlaves(1, "running");
    } else if (btn == 2) {
      sendToSlaves(1, "chase");
    } else if (btn == 3) {
      sendToSlaves(1, "flash");
    } else if (btn == 4) {
      sendToSlaves(1, "alternate");
    }
  } else if (btn >= 5 && btn <= 8) {
    // Slave 2 commands
    if (btn == 5) {
      sendToSlaves(2, "running lights");
    } else if (btn == 6) {
      sendToSlaves(2, "wave");
    } else if (btn == 7) {
      sendToSlaves(2, "blink slow");
    } else if (btn == 8) {
      sendToSlaves(2, "blink fast");
    }
  }
}

void sendToSlaves(int slave, const char* cmd) {
  // SPI
  int ssPin = (slave == 1) ? ss1 : ss2;
  digitalWrite(ssPin, LOW);
  for (int i = 0; cmd[i] != '\0'; i++) {
    SPI.transfer(cmd[i]);
  }
  SPI.transfer('\0');
  digitalWrite(ssPin, HIGH);
}