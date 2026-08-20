#pragma once

// PlantSense AI Configuration
#define PLANT_ID "spiderPlant"

// Data Collection Interval (30 minutes)
#define SENSOR_READ_INTERVAL_MS (30 * 60 * 1000)

// Pins
#define DHT_PIN 4
#define DHT_TYPE DHT11

#define SOIL_POWER_PIN 25
#define SOIL_DATA_PIN 34

// Calibration Values
// You should calibrate these by testing the sensor in dry air and a glass of water
#define SOIL_DRY_VALUE 3500  // High value when dry (ESP32 ADC is 12-bit: 0-4095)
#define SOIL_WET_VALUE 1500  // Low value when wet

#define SERIAL_BAUD_RATE 115200
