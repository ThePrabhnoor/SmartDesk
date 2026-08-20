#include "sensor.h"
#include "config.h"
#include <DHT.h>

DHT dht(DHT_PIN, DHT_TYPE);

void SensorManager::init() {
    pinMode(SOIL_POWER_PIN, OUTPUT);
    digitalWrite(SOIL_POWER_PIN, LOW);
    
    dht.begin();
    Serial.println("Sensors Initialized.");
}

SensorData SensorManager::readAll() {
    SensorData data;
    
    // Read DHT11
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();

    if (isnan(data.temperature) || isnan(data.humidity)) {
        Serial.println("Failed to read from DHT sensor!");
    }
    
    // Read Soil Moisture
    // Power up sensor to prevent corrosion
    digitalWrite(SOIL_POWER_PIN, HIGH);
    delay(200);
    
    data.rawMoisture = analogRead(SOIL_DATA_PIN);
    
    digitalWrite(SOIL_POWER_PIN, LOW); // Power down
    
    // Map raw value to 0-100%
    int moisturePercent = map(data.rawMoisture, SOIL_DRY_VALUE, SOIL_WET_VALUE, 0, 100);
    
    // Clamp values
    if (moisturePercent > 100) moisturePercent = 100;
    if (moisturePercent < 0) moisturePercent = 0;
    
    data.moisturePercent = moisturePercent;
    
    Serial.print("Temp: ");
    Serial.print(data.temperature);
    Serial.print("C, Hum: ");
    Serial.print(data.humidity);
    Serial.print("%, Raw Soil: ");
    Serial.print(data.rawMoisture);
    Serial.print(", Soil %: ");
    Serial.println(data.moisturePercent);
    
    return data;
}
