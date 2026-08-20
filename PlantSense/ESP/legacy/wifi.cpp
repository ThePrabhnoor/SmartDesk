#include "wifi.h"
#include <WiFi.h>
#include "secrets.h"

void WiFiManager::init() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    ensureConnection();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::ensureConnection() {
    if (!isConnected()) {
        Serial.print("WiFi not connected. Reconnecting...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 20) {
            delay(500);
            Serial.print(".");
            retries++;
        }
        if (isConnected()) {
            Serial.println("\nWiFi connected! IP address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nWiFi connection failed.");
        }
    }
}
