const int outputPins[10] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const int NUM_OUTPUTS  = 10;

void setup() {
  Serial.begin(115200);      // more standard baudrate
  Serial1.begin(9600);       // hardware UART on R4 (pins D0=RX, D1=TX)
  for (int i = 0; i < NUM_OUTPUTS; i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }
}

void loop() {
  if (Serial1.available() > 0) {
    char command = Serial1.read();

    // turn all LEDs OFF first
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], LOW);
    }

    // select effect based on command
    if (command == '1') knightRider(3, 100);
    else if (command == '2') pairBlink(3, 200);
    else if (command == '3') centerOut(1, 100);
    else if (command == '4') outsideIn(1, 300);
    else if (command == '5') alternate(3, 500);
    else if (command == '6') randomBlink(10, 100);
    else if (command == '7') wave(1, 100);
    else if (command == '8') strobe(5, 100);

    // turn all LEDs OFF after effect
    for (int i = 0; i < NUM_OUTPUTS; i++) {
      digitalWrite(outputPins[i], LOW);
    }
  }
}
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
      digitalWrite(outputPins[i+3], HIGH);
      delay(delayTime);
      digitalWrite(outputPins[i], LOW);
      digitalWrite(outputPins[i+3], LOW);
    }
  }
}

void centerOut(int repeat, int delayTime) {
  int mid = NUM_OUTPUTS / 2;
  for (int r = 0; r < repeat; r++) {
    for (int i = 0; i <= mid; i++) {
      int left = mid - i;
      int right = mid + i -1;
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
      if (i % 2 == 0) digitalWrite(outputPins[i], HIGH);
      else digitalWrite(outputPins[i], LOW);
    }
    delay(delayTime);

    for (int i = 0; i < NUM_OUTPUTS; i++) {
      if (i % 2 == 1) digitalWrite(outputPins[i], HIGH);
      else digitalWrite(outputPins[i], LOW);
    }
    delay(delayTime);
  }
}


void randomBlink(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    int led = random(NUM_OUTPUTS); // pick random LED
    digitalWrite(outputPins[led], HIGH);
    delay(delayTime);
    digitalWrite(outputPins[led], LOW);
  }
}

void wave(int repeat, int delayTime) {
  for (int r = 0; r < repeat; r++) {
    for (int offset = 0; offset < repeat; offset++) {
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
