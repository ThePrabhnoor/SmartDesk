#pragma once
#include <Arduino.h>

struct SensorData {
    float temperature;
    float humidity;
    int rawMoisture;
    int moisturePercent;
};

class FirebaseManager {
public:
    static void init();
    static bool uploadData(const SensorData& data);
};
