#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Anoman";
const char* password = "12345678";

WiFiUDP udp;
const int receiverPort = 4210;
char incoming[255];

void setup() {
  Serial.begin(9600);

  // Connect to transmitter AP
  WiFi.begin(ssid, password);
  Serial.print("Connecting to AP");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  udp.begin(receiverPort);
  Serial.println("UDP Receiver started...");
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incoming, 255);
    if (len > 0) incoming[len] = 0;

    // incoming format: "DIRECTION,button"
    String msg = String(incoming);
    int commaIndex = msg.indexOf(',');
    String direction = msg.substring(0, commaIndex);
    String buttonState = msg.substring(commaIndex + 1);

    Serial.print("Received direction: ");
    Serial.println(direction);

    // Optional: check specifically for left/right
    if (direction == "LEFT") {
      Serial.println("Joystick is turning LEFT");
    } else if (direction == "RIGHT") {
      Serial.println("Joystick is turning RIGHT");
    } else if (direction == "FORWARD") {
      Serial.println("Joystick is moving FORWARD");
    } else if (direction == "BACKWARD") {
      Serial.println("Joystick is moving BACKWARD");
    } else if (direction == "CENTER") {
      Serial.println("Joystick is at CENTER");
    }

    Serial.print("Button state: ");
    Serial.println(buttonState == "1" ? "PRESSED" : "RELEASED");

    Serial.println("---------------------------");
  }
}