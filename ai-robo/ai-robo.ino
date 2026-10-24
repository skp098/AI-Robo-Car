#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* gemini_api_key = GEMINI_API_KEY;

// Gemini 1.5 Flash endpoint
const char* api_url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.8-flash:generateContent?key=";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected with ");
  Serial.print(ssid);
  Serial.println("Type your prompt in the Serial Monitor and press Send.");
}

void loop() {
  // Check if user has entered data in the Serial Monitor
  if (Serial.available() > 0) {
    String userInput = Serial.readStringUntil('\n');
    userInput.trim();
    
    if (userInput.length() > 0) {
      Serial.println("\nYou: " + userInput);
      askGemini(userInput);
      Serial.println("\nType your next prompt:");
    }
  }
}

void askGemini(String prompt) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Skip SSL certificate validation to save ESP8266 memory
    
    HTTPClient http;
    String full_url = String(api_url) + gemini_api_key;

    http.begin(client, full_url);
    http.setTimeout(30000);
    http.addHeader("Content-Type", "application/json");

    // Construct the JSON payload for the Gemini API
    JsonDocument doc;
    doc["contents"][0]["parts"][0]["text"] = prompt;

    String requestBody;
    serializeJson(doc, requestBody);

    Serial.println("Thinking...");
    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode == 200) {
      String response = http.getString();
      
      // Parse the incoming JSON response
      JsonDocument responseDoc;
      DeserializationError error = deserializeJson(responseDoc, response);
      
      if (!error) {
        String answer = responseDoc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
        Serial.println("Gemini: \n" + answer);
      } else {
        Serial.println("JSON Parse Error.");
      }
    } else {
      Serial.print("HTTP Request Failed. Error Code: ");
      Serial.println(httpResponseCode);
      Serial.println(http.getString());
    }
    http.end();
  } else {
    Serial.println("Error: WiFi Disconnected");
  }
}
