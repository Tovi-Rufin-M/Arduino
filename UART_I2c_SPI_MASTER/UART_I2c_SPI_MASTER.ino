#include <SoftwareSerial.h>
#include <Wire.h>   // I2C
#include <SPI.h>    // SPI

// ---------- UART setup ----------
SoftwareSerial mySerial(2, 3); // RX=2, TX=3

// ---------- I2C setup ----------
#define I2C_SLAVE1 0x08
#define I2C_SLAVE2 0x09

// ---------- SPI setup ----------
const int ss1 = 10;  // Slave 1 CS
const int ss2 = 9;   // Slave 2 CS
const int ss3 = 8;   // Slave 3 CS

// ---------- Buttons ----------
const int buttonPins[8] = {4, 5, 6, 7, A0, A1, A2, A3}; 
// Changed last two to A0, A1 since D9, D10 are used for SPI CS lines

void setup() {
  Serial.begin(600);
  // UART
  mySerial.begin(9600);

  // I2C
  Wire.begin();

  // SPI
  SPI.begin();
  pinMode(ss1, OUTPUT);
  pinMode(ss2, OUTPUT);
  pinMode(ss3, OUTPUT);
  digitalWrite(ss1, HIGH);
  digitalWrite(ss2, HIGH);
  digitalWrite(ss3, HIGH);

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
  } else if (btn >= 1 && btn <= 8) {
    // Slave 3 commands
    if (btn == 1) {
      sendToSlaves(3, "running");
    } else if (btn == 2) {
      sendToSlaves(3, "chase");
    } else if (btn == 3) {
      sendToSlaves(3, "flash");
    } else if (btn == 4) {
      sendToSlaves(3, "alternate");
    } else if (btn == 5) {
      sendToSlaves(3, "running lights");
    } else if (btn == 6) {
      sendToSlaves(3, "wave");
    } else if (btn == 7) {
      sendToSlaves(3, "blink slow");
    } else if (btn == 8) {
      sendToSlaves(3, "blink fast");
    }
  }
}

void sendToSlaves(int slave, const char* cmd) {
  // UART
  mySerial.print("SLV");
  mySerial.print(slave);
  mySerial.print(":");
  mySerial.println(cmd);

  // I2C
  if (slave == 1) {
    Wire.beginTransmission(I2C_SLAVE1);
    Wire.write(cmd);
    Wire.endTransmission();
  } else if (slave == 2) {
    Wire.beginTransmission(I2C_SLAVE2);
    Wire.write(cmd);
    Wire.endTransmission();
  }

  // SPI
  int ssPin = (slave == 1) ? ss1 : ss2;
  digitalWrite(ssPin, LOW);
  for (int i = 0; cmd[i] != '\0'; i++) {
    SPI.transfer(cmd[i]);
  }
  SPI.transfer('\0'); // <-- Add this line
  digitalWrite(ssPin, HIGH);
}
