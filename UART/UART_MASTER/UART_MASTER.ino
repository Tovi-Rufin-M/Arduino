#include <SoftwareSerial.h>

// RX, TX
SoftwareSerial mySerial(2, 3); // (RX=2, TX=3)

// Define button pins
const int button1 = 4;
const int button2 = 5;
const int button3 = 6;
const int button4 = 7;
const int button5 = A0;
const int button6 = A1;
const int button7 = A2;
const int button8 = A3;

void setup() {
  mySerial.begin(9600);    // UART with Slave

  // Configure buttons as inputs with internal pull-up resistors
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
  pinMode(button6, INPUT_PULLUP);
  pinMode(button7, INPUT_PULLUP);
  pinMode(button8, INPUT_PULLUP);
}

void loop() {
  // Check each button. Since pull-up is used, LOW = pressed.

  if (digitalRead(button1) == LOW) {
    mySerial.println("BTN1"); // Send signal to Slave
    delay(200); // debounce delay
  }
  else if (digitalRead(button2) == LOW) {
    mySerial.println("BTN2");
    delay(200);
  }
  else if (digitalRead(button3) == LOW) {
    mySerial.println("BTN3");
    delay(200);
  }
  else if (digitalRead(button4) == LOW) {
    mySerial.println("BTN4");
    delay(200);
  }
  else if (digitalRead(button5) == LOW) {
    mySerial.println("BTN5");
    delay(200);
  }
  else if (digitalRead(button6) == LOW) {
    mySerial.println("BTN6");
    delay(200);
  }
  else if (digitalRead(button7) == LOW) {
    mySerial.println("BTN7");
    delay(200);
  }
  else if (digitalRead(button8) == LOW) {
    mySerial.println("BTN8");
    delay(200);
  }
}
