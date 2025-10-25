/*******************************************************************
    Weather and BART Display for Berkeley/Rockridge

    Shows current weather in Berkeley and next BART departure from
    Rockridge station towards SFO.

    APIs used:
    - Open-Meteo (no key required): https://open-meteo.com
    - BART API (free key required): https://api.bart.gov/api/register.aspx

    https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
 *******************************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include "credentials.h"

// WiFi credentials (from credentials.h)
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// BART API - Get your free key at: https://api.bart.gov/api/register.aspx
const char* bartApiKey = BART_API_KEY;

// Berkeley coordinates
const float latitude = 37.8716;
const float longitude = -122.2727;

TFT_eSPI tft = TFT_eSPI();

String weatherTemp = "";
String weatherDesc = "";

// Store up to 6 upcoming trains
struct BartTrain {
  String time;
  String destination;
  String color;
  int minutes;  // for sorting
};
BartTrain trains[6];
BartTrain tempTrains[20];  // Temporary storage for sorting

unsigned long lastAPICall = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long apiInterval = 120000; // Poll API every 2 minutes
const unsigned long displayInterval = 60000; // Update display every 1 minute

void setup() {
  Serial.begin(115200);

  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Show connecting message
  tft.setCursor(10, 10, 2);
  tft.println("Connecting to WiFi...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(10, 10, 2);
    tft.println("WiFi Failed!");
    return;
  }

  Serial.println("\nWiFi connected!");

  // Initial fetch
  fetchWeather();
  fetchBART();
  lastAPICall = millis();
  lastDisplayUpdate = millis();
  displayData();
}

void loop() {
  unsigned long currentMillis = millis();

  // Fetch new data from APIs every 2 minutes
  if (currentMillis - lastAPICall >= apiInterval) {
    Serial.println("Fetching fresh data from APIs...");
    fetchWeather();
    fetchBART();
    lastAPICall = currentMillis;
    displayData();
  }
  // Update display every 1 minute (subtract 1 from train times)
  else if (currentMillis - lastDisplayUpdate >= displayInterval) {
    Serial.println("Updating display (subtracting 1 minute from times)...");
    // Subtract 1 minute from all train times
    for (int i = 0; i < 6; i++) {
      if (trains[i].minutes > 0) {
        trains[i].minutes--;
        trains[i].time = (trains[i].minutes == 0) ? "Now" : String(trains[i].minutes) + "m";
      } else if (trains[i].minutes == 0) {
        trains[i].minutes = -1;
        trains[i].time = "-1m";
      }
    }
    lastDisplayUpdate = currentMillis;
    displayData();
  }

  delay(1000);
}

void fetchWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Open-Meteo API endpoint for Berkeley
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 4) +
                 "&longitude=" + String(longitude, 4) +
                 "&current=temperature_2m,weather_code&temperature_unit=fahrenheit&timezone=America/Los_Angeles";

    Serial.println("Fetching weather...");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Weather response:");
      Serial.println(payload);

      // Parse JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float temp = doc["current"]["temperature_2m"];
        int weatherCode = doc["current"]["weather_code"];

        weatherTemp = String((int)temp) + "F";
        weatherDesc = getWeatherDescription(weatherCode);

        Serial.println("Temperature: " + weatherTemp);
        Serial.println("Condition: " + weatherDesc);
      } else {
        Serial.println("JSON parse error");
        weatherTemp = "N/A";
        weatherDesc = "Error";
      }
    } else {
      Serial.println("HTTP error: " + String(httpCode));
      weatherTemp = "N/A";
      weatherDesc = "Error";
    }

    http.end();
  }
}

void fetchBART() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // BART API endpoint for Rockridge station
    String url = "https://api.bart.gov/api/etd.aspx?cmd=etd&orig=ROCK&key=" +
                 String(bartApiKey) + "&json=y";

    Serial.println("Fetching BART...");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("BART response:");
      Serial.println(payload);

      // Parse JSON to get next 6 trains from Rockridge
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonArray etds = doc["root"]["station"][0]["etd"];

        // Clear previous trains
        for (int i = 0; i < 6; i++) {
          trains[i].time = "";
          trains[i].destination = "";
          trains[i].color = "";
          trains[i].minutes = 999;
        }

        // Clear temp trains
        for (int i = 0; i < 20; i++) {
          tempTrains[i].time = "";
          tempTrains[i].destination = "";
          tempTrains[i].color = "";
          tempTrains[i].minutes = 999;
        }

        int trainCount = 0;

        // Collect all trains from all destinations
        for (JsonObject etd : etds) {
          String destination = etd["destination"].as<String>();

          JsonArray estimates = etd["estimate"];
          for (JsonVariant est : estimates) {
            if (trainCount >= 20) break;

            String minutesStr = est["minutes"].as<String>();
            String color = est["color"].as<String>();

            int mins = 0;
            if (minutesStr == "Leaving") {
              mins = 0;
            } else {
              mins = minutesStr.toInt();
            }

            tempTrains[trainCount].time = (mins == 0) ? "Now" : minutesStr + "m";
            tempTrains[trainCount].destination = destination;
            tempTrains[trainCount].color = color;
            tempTrains[trainCount].minutes = mins;

            Serial.print("Collected: ");
            Serial.print(tempTrains[trainCount].minutes);
            Serial.print("min to ");
            Serial.print(tempTrains[trainCount].destination);
            Serial.print(" (");
            Serial.print(tempTrains[trainCount].color);
            Serial.println(")");

            trainCount++;
          }
        }

        // Sort trains by departure time (bubble sort)
        for (int i = 0; i < trainCount - 1; i++) {
          for (int j = 0; j < trainCount - i - 1; j++) {
            if (tempTrains[j].minutes > tempTrains[j + 1].minutes) {
              // Swap
              BartTrain temp = tempTrains[j];
              tempTrains[j] = tempTrains[j + 1];
              tempTrains[j + 1] = temp;
            }
          }
        }

        // Copy first 6 to display array
        Serial.println("\nNext 6 trains:");
        for (int i = 0; i < 6 && i < trainCount; i++) {
          trains[i] = tempTrains[i];
          Serial.print(i + 1);
          Serial.print(". ");
          Serial.print(trains[i].time);
          Serial.print(" to ");
          Serial.print(trains[i].destination);
          Serial.print(" (");
          Serial.print(trains[i].color);
          Serial.println(")");
        }

      } else {
        Serial.println("JSON parse error");
        trains[0].time = "Error";
        trains[0].destination = "Parse failed";
        trains[0].minutes = 0;
      }
    } else {
      Serial.println("HTTP error: " + String(httpCode));
      trains[0].time = "Error";
      trains[0].destination = "HTTP failed";
      trains[0].minutes = 0;
    }

    http.end();
  }
}

void displayData() {
  tft.fillScreen(TFT_BLACK);

  // Title - smaller to fit more info
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(5, 3, 2);
  tft.print("Berkeley: ");
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.print(weatherTemp);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(" ");
  tft.println(weatherDesc);

  // BART section - next 6 trains
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(5, 25, 2);
  tft.println("Rockridge BART:");

  // Display 6 trains - more compact spacing
  int yPos = 45;
  for (int i = 0; i < 6; i++) {
    if (trains[i].time != "") {
      // Time in larger font
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setCursor(5, yPos, 2);
      tft.print(trains[i].time);

      // Destination and color
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(50, yPos, 2);

      // Truncate destination if too long
      String dest = trains[i].destination;
      if (dest.length() > 18) {
        dest = dest.substring(0, 18);
      }
      tft.print(dest);

      // Line color indicator
      if (trains[i].color != "") {
        uint16_t lineColor = getLineColor(trains[i].color);
        tft.fillCircle(310, yPos + 6, 5, lineColor);
      }

      yPos += 30;  // Reduced from 42 to 30 to fit 6 trains
    }
  }

  // Last API fetch time
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(5, 230, 1);
  tft.print("API: ");
  tft.print((millis() - lastAPICall) / 1000);
  tft.print("s ago");
}

uint16_t getLineColor(String color) {
  // Convert BART line colors to RGB565
  if (color == "YELLOW") return TFT_YELLOW;
  if (color == "ORANGE") return TFT_ORANGE;
  if (color == "RED") return TFT_RED;
  if (color == "BLUE") return TFT_BLUE;
  if (color == "GREEN") return TFT_GREEN;
  return TFT_WHITE;
}

String getWeatherDescription(int code) {
  // WMO Weather interpretation codes
  if (code == 0) return "Clear";
  if (code <= 3) return "Partly Cloudy";
  if (code <= 48) return "Foggy";
  if (code <= 55) return "Drizzle";
  if (code <= 67) return "Rain";
  if (code <= 77) return "Snow";
  if (code <= 82) return "Showers";
  if (code <= 86) return "Snow Showers";
  if (code <= 99) return "Thunderstorm";
  return "Unknown";
}
