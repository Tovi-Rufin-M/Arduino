const int buttonPins[8] = {A0, A1, A2, A3, 7, 6, 5, 4};
const int numbtn = 8;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);  // Use hardware UART on R4 (pins D0=RX, D1=TX)

  // Configure button pins ONCE
  for (int i = 0; i < numbtn; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);  // active LOW
  }
}

void loop() {
  uart();
}

void uart() {
  for (int i = 0; i < numbtn; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {   // button pressed
      char command = '1' + i;  // map button index to '1'..'8'
      Serial1.write(command);
      delay(200);  // simple debounce
    }
  }
}
