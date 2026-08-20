#include "firebase.h"
#include "secrets.h"
#include "config.h"
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

void FirebaseManager::init() {
    Serial.println("Initializing Firebase...");
    config.database_url = FIREBASE_URL;
    config.signer.tokens.legacy_token = FIREBASE_SECRET;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    // Set database read timeout to 1 minute (max 15 minutes)
    fbdo.setBSSLBufferSize(1024, 1024);
    fbdo.setResponseSize(1024);
    
    Serial.println("Firebase Initialized.");
}

bool FirebaseManager::uploadData(const SensorData& data) {
    if (!Firebase.ready()) {
        Serial.println("Firebase not ready.");
        return false;
    }

    String basePath = String("/plants/") + PLANT_ID;
    
    // Upload current data using updateNode so we don't overwrite recommendations
    FirebaseJson json;
    json.set("temperature", data.temperature);
    json.set("humidity", data.humidity);
    json.set("rawMoisture", data.rawMoisture);
    json.set("moisture", data.moisturePercent);
    json.set("timestamp", ".sv/timestamp"); // Server timestamp
    
    if (Firebase.RTDB.updateNode(&fbdo, (basePath + "/current").c_str(), &json)) {
        Serial.println("Current data uploaded successfully");
    } else {
        Serial.println("Failed to upload current data");
        Serial.println(fbdo.errorReason());
        return false;
    }

    // Push to history
    if (Firebase.RTDB.pushJSON(&fbdo, (basePath + "/history").c_str(), &json)) {
        Serial.println("History data uploaded successfully");
    } else {
        Serial.println("Failed to upload history data");
        Serial.println(fbdo.errorReason());
        return false;
    }

    return true;
}
