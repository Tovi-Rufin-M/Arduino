#include <Wire.h>

// ---------- I2C Slave Address ----------
#define SLAVE1_ADDR 0x08

// ---------- LED Pins ----------
const int ledPins[] = {4, 5, 6, 7, 8};
const int numLeds = 5;

String command = "";   // to store incoming command
bool newCommand = false;

void setup() {
  // Initialize LEDs
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // Start I2C as Slave
  Wire.begin(SLAVE1_ADDR);
  Wire.onReceive(receiveEvent);
  

  Serial.begin(9600);
  Serial.println("Slave1 Ready");

}

void loop() {
  if (newCommand) {
    Serial.print("Received: ");

    if (command == "running") {
      runningLights();
    } else if (command == "chase") {
      chaseLights();
    } else if (command == "flash") {
      flashLights();
    } else if (command == "alternate") {
      alternateLights();
    }
    newCommand = false;
  }
}

// ---------- I2C Receive ----------
void receiveEvent(int bytes) {
  command = "";
  while (Wire.available()) {
    char c = Wire.read();
    command += c;
  }
  newCommand = true;
}

// ---------- LED Patterns ----------
void runningLights() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], HIGH);
    delay(200);
    digitalWrite(ledPins[i], LOW);
  }
}

void chaseLights() {
  for (int i = 0; i < 3; i++) { // repeat 3 times
    for (int i = 0; i < numLeds; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(150);
      digitalWrite(ledPins[i], LOW);
    }
  }
}

void flashLights() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], HIGH);
    }
    delay(200);
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], LOW);
    }
    delay(200);
  }
}

void alternateLights() {
  for (int i = 0; i < 5; i++) {
    // ON: 1 & 3
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[2], HIGH);
    digitalWrite(ledPins[4], HIGH);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[3], LOW);
    delay(300);

    // ON: 2 & 4
    digitalWrite(ledPins[0], LOW);
    digitalWrite(ledPins[2], LOW);
    digitalWrite(ledPins[4], LOW);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[3], HIGH);
    delay(300);
  }
}


