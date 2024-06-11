#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Pixel_8297";
const char* password = "523478kanat";
// const char* serverUrl = "http://192.168.202.188:3000/sensor_data";
const char* serverUrl = "http://34.125.138.217:8090/gateway/smart-greenhouse/arduino/insert-data?";

const int relayExhaust = D1; 
const int relayLighting = D2;
const int relayVentilation = D3; 
const int relayIrrigation = D4; 
const int relayHeating = D5;

unsigned long lastServerResponseTime = 0;
const unsigned long timeoutDuration = 60000; // 1 minute timeout duration

void setup() {
  Serial.begin(9600);
  pinMode(relayVentilation, OUTPUT);
  pinMode(relayExhaust, OUTPUT);
  pinMode(relayLighting, OUTPUT);
  pinMode(relayHeating, OUTPUT);
  pinMode(relayIrrigation, OUTPUT);

  resetRelays(); 
  connectToWiFi();
}

void loop() {
  if (Serial.available()) {
    String jsonBuffer = Serial.readStringUntil('\n');
    
    StaticJsonDocument<200> doc;
    deserializeJson(doc, jsonBuffer);
    
    float humidity = doc["humidity"];
    float temperature = doc["temperature"];
    float co2 = doc["co2"];
    float soil = doc["soil"];
    float lux = doc["lux"];

    sendToServer(humidity, temperature, co2, soil, lux);
  } 
  // Check for timeout
  if (millis() - lastServerResponseTime > timeoutDuration) {
    resetRelays();
    Serial.println("No response from server, relays reset to OFF");
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void sendToServer(float humidity, float temperature, float co2, float soil, float lux) {
  WiFiClient client;
  HTTPClient http;

  String postData = "id=1&temperature=" + String(temperature) + "&co2=" + String(co2) + "&humidityAir=" + String(humidity) + "&light=" + String(lux) + "&humidityGround=" + String(soil);

  Serial.println("Sending HTTP POST request to server...");

  if (http.begin(client, serverUrl)) {
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
      // Reading response from the server
      String response = http.getString();
      Serial.println("Response from server:");
      Serial.println(response);
      
      StaticJsonDocument<200> responseDoc;
      deserializeJson(responseDoc, response);
      
      bool optimalTemperature = responseDoc["optimalTemperature"];
      bool optimalHumidityAir = responseDoc["optimalHumidityAir"];
      bool optimalHumidityGround = responseDoc["optimalHumidityGround"];
      bool optimalLight = responseDoc["optimalLight"];
      bool optimalCarbonDioxide = responseDoc["optimalCarbonDioxide"];

      controlRelays(optimalTemperature, optimalHumidityAir, optimalHumidityGround, optimalLight, optimalCarbonDioxide);

      lastServerResponseTime = millis();
    } else {
      Serial.print("HTTP Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Failed to connect to server");
  }
}

void controlRelays(bool temp, bool humidityAir, bool humidityGround, bool light, bool co2) {
  digitalWrite(relayVentilation, temp ? LOW : HIGH);
  digitalWrite(relayExhaust, co2 ? LOW : HIGH);
  digitalWrite(relayLighting, light ? LOW : HIGH);
  digitalWrite(relayHeating, temp ? LOW : HIGH);
  digitalWrite(relayIrrigation, humidityGround ? LOW : HIGH);
  
  Serial.println("Relays updated based on server response:");
  Serial.print("Ventilation: "); Serial.println(temp ? "ON" : "OFF");
  Serial.print("Exhaust: "); Serial.println(co2 ? "ON" : "OFF");
  Serial.print("Lighting: "); Serial.println(light ? "ON" : "OFF");
  Serial.print("Heating: "); Serial.println(temp ? "ON" : "OFF");
  Serial.print("Irrigation: "); Serial.println(humidityGround ? "ON" : "OFF");
}

void resetRelays() {
  digitalWrite(relayVentilation, HIGH);
  digitalWrite(relayExhaust, HIGH);
  digitalWrite(relayLighting, HIGH);
  digitalWrite(relayHeating, HIGH);
  digitalWrite(relayIrrigation, HIGH);

  Serial.println("All relays reset to OFF");
}
