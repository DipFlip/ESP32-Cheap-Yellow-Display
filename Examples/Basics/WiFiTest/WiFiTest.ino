/*******************************************************************
    WiFi Connection Test for the ESP32 Cheap Yellow Display.

    This example connects to WiFi and displays the connection status
    and IP address on the screen.

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
 *******************************************************************/

#include <WiFi.h>
#include <TFT_eSPI.h>

// WiFi credentials
const char* ssid     = "RossHouse";
const char* password = "gfolp60648";

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); // Landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Display WiFi connection attempt
  tft.setCursor(10, 10, 2);
  tft.println("Connecting to WiFi...");
  tft.setCursor(10, 30, 2);
  tft.println(ssid);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    tft.print(".");
    attempts++;
  }

  // Clear screen
  tft.fillScreen(TFT_BLACK);

  if (WiFi.status() == WL_CONNECTED) {
    // Success!
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());

    // Display success on screen
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(10, 10, 4);
    tft.println("WiFi Connected!");

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 50, 2);
    tft.println("Network:");
    tft.setCursor(10, 70, 2);
    tft.println(ssid);

    tft.setCursor(10, 100, 2);
    tft.println("IP Address:");
    tft.setCursor(10, 120, 2);
    tft.println(WiFi.localIP());

    tft.setCursor(10, 150, 2);
    tft.print("RSSI: ");
    tft.print(WiFi.RSSI());
    tft.println(" dBm");

  } else {
    // Connection failed
    Serial.println("\nWiFi connection failed!");

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 10, 4);
    tft.println("WiFi Failed!");

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 60, 2);
    tft.println("Could not connect to:");
    tft.setCursor(10, 80, 2);
    tft.println(ssid);
  }
}

void loop() {
  // Nothing to do here
}
