#pragma once
#include <Arduino.h>

class WiFiManager {
public:
    static void init();
    static bool isConnected();
    static void ensureConnection();
};
