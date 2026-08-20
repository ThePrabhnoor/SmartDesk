#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <SinricPro.h>
#include <SinricProTemperaturesensor.h>
#include <SinricProSwitch.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// ==========================================
// WIFI CREDENTIALS
// ==========================================
#define WIFI_SSID "Sidhu_1"
#define WIFI_PASSWORD "Catapult@12"

// ==========================================
// FIREBASE CREDENTIALS
// ==========================================
#define API_KEY "AIzaSyDLo9IsiIVdIMZlQqz8JEVhRrUZt5BHAQw"
#define DATABASE_URL "https://agrosense-e00de-default-rtdb.firebaseio.com"

// ==========================================
// GEMINI AI CREDENTIALS
// ==========================================
#define GEMINI_API_KEY "YOUR_GEMINI_API_KEY"
#define GEMINI_ENDPOINT "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash:generateContent?key=" GEMINI_API_KEY

// ==========================================
// SINRIC PRO CREDENTIALS (GOOGLE HOME)
// ==========================================
#define APP_KEY           "78749eeb-b3eb-48dd-a563-de7ff68a9f8c"
#define APP_SECRET        "5b4af8f9-c8b4-4f25-bac9-b2d9dae931ed-21f84c6b-cea4-4ae5-b367-930339643794"
#define TEMP_SENSOR_ID    "6a670f216ba33a80b99a5525"
#define LAMP_DEVICE_ID    "6a7b5e5c969af7ec2482e176"
#define BAUD_RATE         115200

// ==========================================
// PINS & HARDWARE SETTINGS
// ==========================================
#define DHTPIN 4          // DHT11 data pin connected to GPIO 4
#define DHTTYPE DHT11     // DHT 11 sensor type
#define SOIL_MOISTURE_PIN 34  // Analog pin connected to soil moisture sensor
#define SENSOR_POWER_PIN 25    // GPIO pin to power soil moisture sensor (prevents corrosion)
#define RELAY_PIN 23      // GPIO pin to control the lamp relay

bool lampState = false;   // Current state of the lamp

DHT dht(DHTPIN, DHTTYPE);

// Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long historyLogPrevMillis = 0;
unsigned long aiUpdatePrevMillis = 0;

// Timer delays
const long SEND_DATA_DELAY = 5000;       // Send live data every 5 seconds
const long HISTORY_LOG_DELAY = 300000;   // Push to history every 5 minutes (300,000 ms)
const long AI_UPDATE_DELAY = 900000;     // Run Gemini AI every 15 minutes (900,000 ms)

WiFiClientSecure clientSecure;

void setupSinricPro() {
  SinricProTemperaturesensor &mySensor = SinricPro[TEMP_SENSOR_ID];
  
  SinricProSwitch& mySwitch = SinricPro[LAMP_DEVICE_ID];
  mySwitch.onPowerState([](const String &deviceId, bool &state) {
    Serial.printf("Device %s turned %s (via SinricPro)\r\n", deviceId.c_str(), state ? "on" : "off");
    lampState = state;
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW); // Active High relay logic
    
    // Sync state to Firebase immediately
    if (Firebase.ready()) {
      FirebaseJson fbJson;
      fbJson.set("lamp_state", lampState ? "ON" : "OFF");
      Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &fbJson);
    }
    return true; 
  });

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void callGeminiAI(float t, float h, int moisturePercent) {
  Serial.println("Asking Gemini AI for plant advice...");
  
  clientSecure.setInsecure(); // Do not verify SSL cert
  HTTPClient http;
  http.begin(clientSecure, GEMINI_ENDPOINT);
  http.setTimeout(30000); // 30 seconds timeout for Gemini API
  http.addHeader("Content-Type", "application/json");

  // Construct prompt
  String prompt = "You are a botanist analyzing a Spider Plant. Current Temp: " + String(t) + "C, Humidity: " + String(h) + "%, Soil Moisture: " + String(moisturePercent) + "%. Spider plants need water when moisture drops below 40%. Respond ONLY in valid JSON format: {\\\"healthStatus\\\": \\\"Healthy\\\"|\\\"Needs Attention\\\"|\\\"Critical\\\", \\\"recommendedWaterML\\\": 0, \\\"nextWateringEstimateHours\\\": 24, \\\"recommendationText\\\": \\\"short string\\\"}";
  
  String payload = "{\"contents\":[{\"parts\":[{\"text\":\"" + prompt + "\"}]}]}";
  
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Gemini Raw Response: " + response);
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
        String aiText = doc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
        
        if (aiText != "null" && aiText.length() > 5) {
            // Sometimes Gemini wraps the output in markdown code blocks
            aiText.replace("```json\\n", "");
            aiText.replace("```json", "");
            aiText.replace("```\\n", "");
            aiText.replace("```", "");
            
            Serial.println("Parsed AI JSON: " + aiText);
            
            // Push AI recommendation to Firebase
            FirebaseJson fbJson;
            fbJson.setJsonData(aiText);
            Firebase.RTDB.setJSON(&fbdo, "sensors/plantsense/recommendation", &fbJson);
        } else {
            Serial.println("Gemini returned invalid or empty JSON!");
        }
    } else {
        Serial.print("Failed to parse Gemini response: ");
        Serial.println(error.c_str());
    }
  } else {
    Serial.printf("Error calling Gemini API: %d\n", httpResponseCode);
  }
  http.end();
}

void setup() {
  Serial.begin(BAUD_RATE);
  
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, LOW);
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure lamp is OFF on boot
  
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Setup Firebase
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Auth Successful");
  } else {
    Serial.printf("Firebase Auth Error: %s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Setup SinricPro for Google Home
  setupSinricPro();
}

void loop() {
  // Handle SinricPro tasks
  SinricPro.handle();

  if (Firebase.ready() && (millis() - sendDataPrevMillis > SEND_DATA_DELAY || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    // 1. Read DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      h = 0;
      t = 0;
    }

    // 2. Read Soil Moisture
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(10); // allow power to settle
    int moistureRaw = analogRead(SOIL_MOISTURE_PIN);
    digitalWrite(SENSOR_POWER_PIN, LOW);
    
    int moisturePercent = map(moistureRaw, 3000, 1000, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.printf("Temp: %.1fC, Hum: %.1f%%, Moisture: %d%%\n", t, h, moisturePercent);

    // 3. Prepare JSON for Firebase
    FirebaseJson json;
    json.set("air_temp", t);
    json.set("air_humidity", h);
    json.set("soil_moisture", moisturePercent);
    json.set("lamp_state", lampState ? "ON" : "OFF");
    json.set("timestamp", ".sv/timestamp");

    // 4. Update the current state node without erasing the recommendation object
    Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &json);

    // 5. Check if it's time to log historical data
    if (millis() - historyLogPrevMillis > HISTORY_LOG_DELAY || historyLogPrevMillis == 0) {
       historyLogPrevMillis = millis();
       Firebase.RTDB.pushJSON(&fbdo, "sensors/plantsense_history", &json);
    }
    
    // 6. Ask Gemini AI for recommendations (Runs every 15 minutes)
    if (millis() - aiUpdatePrevMillis > AI_UPDATE_DELAY || aiUpdatePrevMillis == 0) {
       aiUpdatePrevMillis = millis();
       callGeminiAI(t, h, moisturePercent);
    }

    // 7. Check for Lamp Commands from Custom App (via Firebase)
    if (Firebase.RTDB.getString(&fbdo, "sensors/plantsense/lamp_command")) {
      String command = fbdo.stringData();
      if ((command == "ON" && !lampState) || (command == "OFF" && lampState)) {
        lampState = (command == "ON");
        digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);
        
        // Sync state back to SinricPro
        SinricProSwitch& mySwitch = SinricPro[LAMP_DEVICE_ID];
        mySwitch.sendPowerStateEvent(lampState);
        
        Serial.printf("Lamp turned %s (via Firebase Command)\r\n", lampState ? "on" : "off");
      }
      // Consume the command
      Firebase.RTDB.deleteNode(&fbdo, "sensors/plantsense/lamp_command");
    }

    // 8. Send data to Sinric Pro (Google Home)
    if (!isnan(t) && !isnan(h) && t != 0) {
      SinricProTemperaturesensor &mySensor = SinricPro[TEMP_SENSOR_ID];
      mySensor.sendTemperatureEvent(t, h);
    }
  }
}
