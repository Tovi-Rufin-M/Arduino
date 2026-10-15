#include <WiFiS3.h>
#include <WiFiUdp.h>

// --- Wi-Fi AP Settings ---
char ssid[] = "ArduinoUnoAP"; // The name for your Access Point
char pass[] = "12345678";     // Password (must be 8+ characters)
int status = WL_IDLE_STATUS;

// --- UDP Settings ---
unsigned int localPort = 4210; // Port to listen on
WiFiUDP Udp;                   // UDP instance

// --- Packet Buffer ---
char packetBuffer[255]; // Buffer to hold incoming packet data

//pins
const int led1 = 12;
const int led2 = 11;
const int led3 = 10;
const int led4 = 8;

// Motor pins
const int IN1 = 3;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 9;

void setup() {
  // Start Serial for monitoring
  Serial.begin(9600);
  Serial1.begin(19200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  Serial.println("Starting AP (Access Point)...");

  // Start the Access Point
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true); // Don't continue
  }

  Serial.println("AP Started.");
  
  // Print AP IP address
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  // Start listening for UDP packets
  Serial.print("Starting UDP listener on port ");
  Serial.println(localPort);
  if (!Udp.begin(localPort)) {
    Serial.println("Failed to start UDP listener.");
    while(true); // Don't continue
  }
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    int len = Udp.read(packetBuffer, 255);

    
    if (len > 0) packetBuffer[len] = '\0';

    Serial.println(packetBuffer);
    resive(packetBuffer);
    Serial.println("end loop");
    // --- FIX: Send reply back to sender ---
  }
}

void resive(String packetBuffer) {
  if (Serial1.available()) {
    String incoming = Serial1.readStringUntil('\n');  // Read full line

    // Extract the STOP part after the last comma
    int stopStart = incoming.lastIndexOf(",") + 1;  
    String stopVal = incoming.substring(stopStart);

    Serial.print("R3 -> ");
    Serial.println(incoming);
    stopVal.trim();  // Removes leading/trailing whitespace and newlines
    if (stopVal == "STOP"){
      carmove("BACKWARD");
      Serial.println("STOP value: " + stopVal);
    } else {
      carmove(packetBuffer);
    }

    send(incoming);
  }
}


void carmove(String direction) {
  if (direction == "FORWARD") {
    // Both motors forward
    analogWrite(IN1, 0);
    analogWrite(IN2, 200);
    analogWrite(IN3, 200);
    analogWrite(IN4, 0);
    Serial.println("deretso");
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  }
  else if (direction == "BACKWARD") {
    analogWrite(IN1, 200);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 200);
    Serial.println("atras");
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
  }
  else if (direction == "LEFT") {
    // Left turn: left motor backward, right motor forward
    analogWrite(IN1, 0);
    analogWrite(IN2, 200);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    Serial.println("pihak");
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  }
  else if (direction == "RIGHT") {
    // Right turn: left motor forward, right motor backward
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 200);
    analogWrite(IN4, 0);
    Serial.println("hakpi");
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
  }
  else if (direction == "STOP") {
    // Stop motors
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    Serial.println("untat");
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, HIGH);

  }
}

void send(String data){
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(data.c_str());   // <--- FIX: convert String to char*
    Udp.endPacket();
}
