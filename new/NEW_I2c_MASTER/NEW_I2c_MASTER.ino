// Pin definitions
int leftMotorPin1 = 2;
int leftMotorPin2 = 3;
int rightMotorPin1 = 5;
int rightMotorPin2 = 4;

void setup() {
  // Set all motor pins as output
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
}

void loop() {
  // Step 1: Spin left motor
  digitalWrite(leftMotorPin1, HIGH);
  digitalWrite(rightMotorPin1, HIGH);

  // Step 2: Spin right motor
  digitalWrite(rightMotorPin2, 0);
  digitalWrite(leftMotorPin2, 0);
  // Stop right moto
}
