#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#include "GxEPD2_display_selection_new_style.h"

const char* ssid = "";
const char* password = "";
String newHostName = "ESP_1_54_Paper";

const char* outside = "";
const char* inside = "";
const char* box = "";

const String tempOutsideStr = "A:";
const String tempInsideStr = "I:";
const String tempBoxStr = "B:";
const String tempStr = "-Temp:";
const String humidityStr = "-HUM:";
const String pressureStr = "-PSR:";

int count = 0;

struct Weather {
  float Temperature;
  float Humidity;
  float Pressure;
};

void printValues();
float fetchTemperature(const char* url);
Weather fetchWeather(const char* url);

void setup() {
  display.init(115200, true, 2, false); 
  Serial.println("");

  WiFi.mode(WIFI_STA);
  WiFi.hostname(newHostName.c_str());
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.setRotation(3);
  display.setFont(&FreeMonoBold18pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setTextSize(1);

  printValues();
}

void loop() {
  delay(10000);
  count++;

  if (count >= 6 * 15) {
    printValues();
    count = 0;
  }
}

void printValues() {
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(tempStr, 0, 0, &tbx, &tby, &tbw, &tbh);

  float tempOutside = fetchTemperature(outside);
  Serial.print("Temperature outside: ");
  Serial.println(tempOutside);

  float tempInside = fetchTemperature(inside);
  Serial.print("Temperature outside: ");
  Serial.println(tempInside);

  Weather weatherBox = fetchWeather(box);
  Serial.print("Temperature Box: ");
  Serial.println(weatherBox.Temperature);
  Serial.print("Humidity Box: ");
  Serial.println(weatherBox.Humidity);
  Serial.print("Pressure Box: ");
  Serial.println(weatherBox.Pressure);
  
  display.setFullWindow();
  display.firstPage();

  do
  {
    display.fillScreen(GxEPD_WHITE);
    
    int offset = 22;
    
    display.setCursor(0, offset);
    display.print(tempOutsideStr);
    String tempOutsideDisplay = String(tempOutside, 1) + "C";
    display.getTextBounds(tempOutsideDisplay, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) - 5, offset);
    display.print(tempOutsideDisplay);

    offset += 15;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);

    offset += 35;

    display.setCursor(0, offset);
    display.print(tempInsideStr);
    String tempInsideDisplay = String(tempInside, 1) + "C";
    display.getTextBounds(tempInsideDisplay, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) - 5, offset);
    display.print(tempInsideDisplay);

    offset += 15;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);
    offset++;
    display.drawLine(0, offset, 200, offset, GxEPD_BLACK);

    offset += 35;

    display.setCursor(0, offset);
    display.print(tempBoxStr);

    //display.setCursor(0, 92);
    //display.print(tempStr);
    String tempBoxDisplay = String(weatherBox.Temperature, 1) + "C";
    display.getTextBounds(tempBoxDisplay, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) - 5, offset);
    display.print(tempBoxDisplay);

    //display.setCursor(0, 118);
    //display.print(humidityStr);
    offset += 35;

    String humidityDisplay = String(weatherBox.Humidity, 1) + "%";
    display.getTextBounds(humidityDisplay, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) - 5, offset);
    display.print(humidityDisplay);

    //display.setCursor(0, 144);
    //display.print(pressureStr);
    offset += 35;

    String pressureDisplay = String(weatherBox.Pressure / 10, 1) + "hPa";
    display.getTextBounds(pressureDisplay, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor((display.width() - tbw) - 5, offset);
    display.print(pressureDisplay);
  }
  while (display.nextPage());
}

float fetchTemperature(const char* url) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();
  float temperature = -128;

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        temperature = doc["temperatureInC"];
      } else {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
      }
    }
  } else {
    Serial.print("GET request failed: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
  return temperature;
}

Weather fetchWeather(const char* url) {
  WiFiClient client;
  HTTPClient http;
  Weather weather;
  http.begin(client, url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        weather.Temperature = doc["temperatureInC"];
        weather.Humidity = doc["humidityPerc"];
        weather.Pressure = doc["pressureInPa"];
        
      } else {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
      }
    }
  } else {
    Serial.print("GET request failed: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
  return weather;  
}

