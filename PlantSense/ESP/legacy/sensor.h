#pragma once
#include <Arduino.h>
#include "firebase.h" // For SensorData struct

class SensorManager {
public:
    static void init();
    static SensorData readAll();
};
