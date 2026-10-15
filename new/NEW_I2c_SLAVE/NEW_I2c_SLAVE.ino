#include <Wire.h>

const int outputPins[8] = {2,3,4, 5, 6, 7, 8, 9};
const int NUM_OUTPUTS = 8;

// ---------- I2C setup ----------
#define SLAVE_ADDR 0x09   // must match master's address

void setup() {
  Serial.begin(115200);

  // Set up LEDs
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  // Join I2C bus as slave
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);   // function to handle incoming data
  Serial.println("I2C Slave ready.");
}

void loop() {
  // Nothing in main loop – everything handled by I2C event
}

// ---------- I2C receive handler ----------
void receiveEvent(int howMany) {
  while (Wire.available()) {
    char command = Wire.read();

    // Turn off all LEDs first
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], LOW);
    }

    // Select effect based on command
    if (command == '1') knightRider(3, 100);
    else if (command == '2') pairBlink(3, 150);
    else if (command == '3') centerOut(1, 100);
    else if (command == '4') outsideIn(1, 100);
    else if (command == '5') alternate(3, 200);
    else if (command == '6') randomBlink(10, 100);
    else if (command == '7') wave(1, 100);
    else if (command == '8') strobe(5, 100);

    // Turn all LEDs off after effect
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], LOW);
    }
    break;
  }
}

// ---------- LED Patterns ----------
void knightRider(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], HIGH);
      delay(delayTime);
      digitalWrite(outputPins[i], LOW);
    }
    for (int i = NUM_OUTPUTS - 2; i > 0; i--) {
      digitalWrite(outputPins[i], HIGH);
      delay(delayTime);
      digitalWrite(outputPins[i], LOW);
    }
  }
}

void pairBlink(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i < NUM_OUTPUTS - 1; i++) {
      digitalWrite(outputPins[i], HIGH);
      digitalWrite(outputPins[i+3 < NUM_OUTPUTS ? i+3 : NUM_OUTPUTS-1], HIGH);
      delay(delayTime);
      digitalWrite(outputPins[i], LOW);
      digitalWrite(outputPins[i+3 < NUM_OUTPUTS ? i+3 : NUM_OUTPUTS-1], LOW);
    }
  }
}

void centerOut(int repeat, int delayTime) {
  int mid = NUM_OUTPUTS / 2;
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i <= mid; i++) {
      int left = mid - i;
      int right = mid + i - 1;
      if (left >= 0) digitalWrite(outputPins[left], HIGH);
      if (right < NUM_OUTPUTS) digitalWrite(outputPins[right], HIGH);
      delay(delayTime);
      if (left >= 0) digitalWrite(outputPins[left], LOW);
      if (right < NUM_OUTPUTS) digitalWrite(outputPins[right], LOW);
    }
  }
}

void outsideIn(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i <= NUM_OUTPUTS / 2; i++) {
      int left = i;
      int right = NUM_OUTPUTS - 1 - i;
      digitalWrite(outputPins[left], HIGH);
      digitalWrite(outputPins[right], HIGH);
      delay(delayTime);
      digitalWrite(outputPins[left], LOW);
      digitalWrite(outputPins[right], LOW);
    }
  }
}

void alternate(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], (i % 2 == 0) ? HIGH : LOW);
    }
    delay(delayTime);

    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], (i % 2 == 1) ? HIGH : LOW);
    }
    delay(delayTime);
  }
}

void randomBlink(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    int led = random(NUM_OUTPUTS);
    digitalWrite(outputPins[led], HIGH);
    delay(delayTime);
    digitalWrite(outputPins[led], LOW);
  }
}

void wave(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int offset = 0; offset < NUM_OUTPUTS; offset++) {
      for (int i = 0; i < NUM_OUTPUTS; i++) {
        int pos = (i + offset) % NUM_OUTPUTS;
        digitalWrite(outputPins[pos], HIGH);
        delay(delayTime);
        digitalWrite(outputPins[pos], LOW);
      }
    }
  }
}

void strobe(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], HIGH);
    }
    delay(delayTime);
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], LOW);
    }
    delay(delayTime);
  }
}
