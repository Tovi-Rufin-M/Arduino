#include <Wire.h>

// ---------- I2C Slave Address ----------
#define SLAVE2_ADDR 0x09   // Different from Slave1

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
  Wire.begin(SLAVE2_ADDR);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  Serial.println("Slave2 Ready");
}

void loop() {
  if (newCommand) {
    Serial.print("Received: ");

    if (command == "running lights") {
      runningLights();
    } else if (command == "wave") {
      waveLights();
    } else if (command == "blink slow") {
      blinkLights(500);
    } else if (command == "blink fast") {
      blinkLights(150);
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

void waveLights() {
  for (int i = 0; i < 3; i++) { 
    // forward
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], HIGH);
      delay(200);
      digitalWrite(ledPins[j], LOW);
    }
    // backward
    for (int j = numLeds - 1; j >= 0; j--) {
      digitalWrite(ledPins[j], HIGH);
      delay(200);
      digitalWrite(ledPins[j], LOW);
    }
  }
}

void blinkLights(int speed) {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], HIGH);
    }
    delay(speed);
    for (int j = 0; j < numLeds; j++) {
      digitalWrite(ledPins[j], LOW);
    }
    delay(speed);
  }
}
