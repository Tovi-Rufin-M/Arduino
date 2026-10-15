#include <WiFiS3.h>
#include <WiFiUdp.h>

// --- Wi-Fi AP Settings ---
char ssid[] = "ArduinoUnoAP"; // The AP name (from the other board)
char pass[] = "12345678";     // The AP password
int status = WL_IDLE_STATUS;
// --- UDP Settings ---
// The AP's IP is typically 192.168.4.1 by default
IPAddress serverIP(192, 168, 4, 1); 
unsigned int serverPort = 4210; // Port to send to (must match receiver)
WiFiUDP Udp;                    // UDP instance

//pins
const int joyX = A0;
const int joyY = A1;
const int joySW = 2;  // switch button

//display
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 64, &Wire, -1);

//buffer
char packetBuffer[255]; // Buffer to hold incoming packet data

void setup() {
  // Start Serial for monitoring
  Serial.begin(9600);
  while (!Serial) {;}
  // Check for Wi-Fi module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
  Serial.println("Attempting to connect to AP...");
  // Connect to the Wi-Fi AP
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    Serial.print(".");
    delay(1000); // Wait 1 second for connection
  }
  Serial.println("\nConnected to AP!");
  
  // Print this board's IP
  IPAddress ip = WiFi.localIP();
  Serial.print("My IP address: ");
  Serial.println(ip);

  // Start UDP
  if (!Udp.begin(8888)) { // Start UDP on an arbitrary local port
    Serial.println("Failed to start UDP.");
    while(true);
  }
  Serial.println("UDP Started.");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("READY");
  display.display();

  pinMode(joySW, INPUT_PULLUP);
}

void loop() {
  // Start the UDP packet
  Udp.beginPacket(serverIP, serverPort);
  display.clearDisplay();
  control();
  
  // End and send the packet
  if (!Udp.endPacket()) {
    Serial.println("Error sending packet!");
  }
  // Wait 1 seconds before sending again
  resive();
  display.display();
  delay(1000);
}

void resive() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {

    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = '\0';

    Serial.println(packetBuffer);

    // Convert buffer to Arduino String
    String data = String(packetBuffer);

    // --- Parsing ---
    int ldrStart = data.indexOf("LDR:") + 4;
    int ldrEnd   = data.indexOf(",", ldrStart);
    String ldrVal = data.substring(ldrStart, ldrEnd);
 
    int tStart = data.indexOf("T:") + 2;
    int tEnd   = data.indexOf(",", tStart);
    String tempVal = data.substring(tStart, tEnd);

    int stopStart = data.lastIndexOf(",") + 1;  // Position after last comma
    String stopVal = data.substring(stopStart);  // Read until the end

    // --- OLED Display ---
    display.setTextSize(1);
    display.setCursor(0, 0);  
    display.print("LDR: ");
    display.println(ldrVal);

    display.setCursor(0, 15);
    display.print("Temp: ");
    display.println(tempVal);

    display.setCursor(0, 30);
    display.print("Dist: ");
    display.println(stopVal);
  }
}


void control() {
  int xVal = analogRead(joyX);
  int yVal = analogRead(joyY);
  int swVal = digitalRead(joySW);

  String direction;

  if (yVal < 400)      direction = "LEFT";
  else if (yVal > 600) direction = "RIGHT";
  else if (xVal < 400) direction = "BACKWARD";
  else if (xVal > 600) direction = "FORWARD";
  else                 direction = "STOP";

  Udp.write(direction.c_str());
  if (swVal == LOW) Udp.write("BTN");

  // --- OLED Display ---
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.println(direction);
  display.setCursor(70, 45);
  display.println(swVal == LOW ? "BTN PRESS" : "BTN OFF");
}

