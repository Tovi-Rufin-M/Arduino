// Slave UART (each listens to its UART port)
// Connect LED to pin 8

const int led = 8;

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(9600);  // Each slave connected to master's Serial1 or Serial2
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd == "BLINK") {
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
  }
}
