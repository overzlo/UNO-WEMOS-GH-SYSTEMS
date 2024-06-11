#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ135.h>
#include <Wire.h>
#include <BH1750.h> 

#define DHTPIN 2    
#define DHTTYPE DHT11  
#define MQ135PIN A0   
#define SOIL_MOISTURE_SENSOR_PIN A1

DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor(MQ135PIN);
BH1750 lightMeter;

void setup() {
  Serial.begin(9600);
  dht.begin();
  Wire.begin();
  lightMeter.begin();
}

void loop() {
  float humidity = dht.readHumidity(); 
  float temperature = dht.readTemperature(); 

  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
  float resistance = mq135_sensor.getResistance();
  float ppm = mq135_sensor.getPPM();
  float co2 = mq135_sensor.getCorrectedPPM(temperature, humidity);

  int soilMoistureValue = analogRead(SOIL_MOISTURE_SENSOR_PIN);
  int soilMoisturePercent = map(soilMoistureValue, 0, 1023, 100,0);
  
  float lux = lightMeter.readLightLevel();

  DynamicJsonDocument doc(200); 
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["co2"] = co2;
  doc["soil"] = soilMoisturePercent;
  doc["lux"] = lux;

  serializeJson(doc, Serial); 
  Serial.println(); 

  delay(5000); 
}

