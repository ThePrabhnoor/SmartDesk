#include <Arduino.h>
#include "config.h"
#include "wifi.h"
#include "firebase.h"
#include "sensor.h"

unsigned long lastReadTime = 0;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial) { ; }
    
    Serial.println("Starting PlantSense AI...");
    
    SensorManager::init();
    WiFiManager::init();
    FirebaseManager::init();
}

void loop() {
    WiFiManager::ensureConnection();
    
    unsigned long currentMillis = millis();
    if (currentMillis - lastReadTime >= SENSOR_READ_INTERVAL_MS || lastReadTime == 0) {
        lastReadTime = currentMillis;
        
        Serial.println("--- Taking Measurements ---");
        SensorData data = SensorManager::readAll();
        
        if (!isnan(data.temperature) && !isnan(data.humidity)) {
            FirebaseManager::uploadData(data);
        } else {
            Serial.println("Skipping upload due to invalid sensor data.");
        }
    }
    
    delay(1000); // Small delay to prevent tight loop
}
