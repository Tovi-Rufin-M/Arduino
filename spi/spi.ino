#include <SPI.h>

volatile boolean receivedFlag = false;
volatile char receivedChar;
char buffer[32];   // Buffer for incoming command
byte index = 0;


// ---------- LED Pins ----------
const int ledPins[] = {4, 5, 6, 7, 8};
const int numLeds = 5;


void setup() {
  Serial.begin(9600);

  // Set MISO as output (required for SPI slave)
  pinMode(MISO, OUTPUT);

  // Enable SPI in slave mode
  SPCR |= _BV(SPE);

  // Enable SPI interrupt
  SPI.attachInterrupt();

  Serial.println("SPI Slave ready (UNO R3)");
}

ISR(SPI_STC_vect) {
  // Interrupt triggered when a byte is received
  receivedChar = SPDR;

  if (receivedChar == '\0' || index >= sizeof(buffer) - 1) {
    buffer[index] = '\0';  // End string
    receivedFlag = true;
    index = 0;  // Reset buffer index
  } else {
    buffer[index++] = receivedChar;
  }
}

void loop() {
  if (receivedFlag) {
    receivedFlag = false;

    Serial.print("Received command: ");
    Serial.println(buffer);

    // Example: react to commands
    if (strcmp(buffer, "running") == 0) {
      runningLights();
    } else if (strcmp(buffer, "chase") == 0) {
      chaseLights();
    } else if (strcmp(buffer, "flash") == 0) {
      flashLights();
    } else if (strcmp(buffer, "alternate") == 0) {
      alternateLights();
    } else {
      Serial.println("-> Unknown command");
    }
  }
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
  for (int j = 0; j < numLeds; j++) {
    digitalWrite(ledPins[j], LOW);
  }
}