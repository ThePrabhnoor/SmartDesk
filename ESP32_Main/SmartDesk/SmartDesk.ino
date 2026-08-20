#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Matter.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProTemperaturesensor.h>

/* Firebase ESP Client Addons */
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "secrets.h"

// --- Hardware Pin Assignments ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 34
#define SENSOR_POWER_PIN 25
#define RELAY_PIN 23
#define BUTTON_PIN 32
#define BAUD_RATE 115200

// --- Application Timers ---
#define SEND_DATA_DELAY 5000
#define HISTORY_LOG_DELAY 300000

// --- SinricPro Constants ---
#define TEMP_SENSOR_ID    "6a670f216ba33a80b99a5525"
#define LAMP_DEVICE_ID    "6a7b5e5c969af7ec2482e176"
#define SOIL_SENSOR_ID    "6a7e1a0c969af7ec2485088d"

// --- Globals ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
unsigned long historyLogPrevMillis = 0;
bool lampState = false;
bool printedCommissioning = false;

// --- SinricPro Rate Limiting State ---
unsigned long lastSinricProEventMillis = 0;
float lastSentT = -100;
float lastSentH = -100;
int lastSentMoisture = -100;

// --- Physical Button Debounce State ---
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

DHT dht(DHTPIN, DHTTYPE);

// Matter Endpoints
MatterOnOffLight matterLamp;
MatterTemperatureSensor matterTempSensor;
MatterHumiditySensor matterHumidSensor;

bool getLampState() { return lampState; }

void setLampState(bool state) {
    lampState = state;
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);
}

bool onPowerState(const String &deviceId, bool &state) {
    Serial.printf("Device %s turned %s (via SinricPro)\r\n", deviceId.c_str(), state ? "on" : "off");
    setLampState(state);
    
    // Sync Matter and Firebase
    matterLamp.setOnOff(state);
    if (Firebase.ready()) {
        FirebaseJson fbJson;
        fbJson.set("lamp_state", state ? "ON" : "OFF");
        Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &fbJson);
    }
    return true; 
}

void setupSmartLamp() {
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    setLampState(false);
}

void setupPlantSense() {
    pinMode(SENSOR_POWER_PIN, OUTPUT);
    digitalWrite(SENSOR_POWER_PIN, LOW);
    dht.begin();
}

void readAndSendPlantData(FirebaseData& fbdo, bool currentLampState) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT sensor!");
        h = 0;
        t = 0;
    }

    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(500); // 500ms stabilization delay for capacitive sensor
    int moistureRaw = analogRead(SOIL_MOISTURE_PIN);
    digitalWrite(SENSOR_POWER_PIN, LOW);
    
    int moisturePercent = map(moistureRaw, 4095, 1500, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.printf("Temp: %.1fC, Hum: %.1f%%, Moisture: %d%% (Raw Analog: %d)\n", t, h, moisturePercent, moistureRaw);

    // Update Matter Endpoints
    if (t != 0) matterTempSensor.setTemperature(t);
    if (h != 0) matterHumidSensor.setHumidity(h);

    FirebaseJson json;
    json.set("air_temp", t);
    json.set("air_humidity", h);
    json.set("soil_moisture", moisturePercent);
    json.set("lamp_state", currentLampState ? "ON" : "OFF");
    json.set("timestamp", ".sv/timestamp");

    Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &json);

    if (millis() - historyLogPrevMillis > HISTORY_LOG_DELAY || historyLogPrevMillis == 0) {
       historyLogPrevMillis = millis();
       Firebase.RTDB.pushJSON(&fbdo, "sensors/plantsense_history", &json);
    }
    
    // Transmit telemetry to SinricPro
    if (!isnan(t) && !isnan(h) && t != 0) {
      if (millis() - lastSinricProEventMillis > 60000 || lastSentT == -100 || 
          abs(t - lastSentT) >= 0.5 || abs(h - lastSentH) >= 1.0 || abs(moisturePercent - lastSentMoisture) >= 2) {
          
          SinricProTemperaturesensor &mySensor = SinricPro[TEMP_SENSOR_ID];
          mySensor.sendTemperatureEvent(t, h);
          
          SinricProTemperaturesensor &soilSensor = SinricPro[SOIL_SENSOR_ID];
          soilSensor.sendTemperatureEvent((float)moisturePercent, (float)moisturePercent);
          
          lastSentT = t;
          lastSentH = h;
          lastSentMoisture = moisturePercent;
          lastSinricProEventMillis = millis();
          Serial.println("Pushed updated sensor data to SinricPro.");
      }
    }
}

void setup() {
  Serial.begin(BAUD_RATE);
  
  setupSmartLamp();
  setupPlantSense();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  matterLamp.begin();
  matterTempSensor.begin();
  matterHumidSensor.begin();
  Matter.begin();

  ArduinoOTA.setHostname("SmartDesk-ESP32");
  ArduinoOTA.begin();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = FIREBASE_USER_EMAIL;
  auth.user.password = FIREBASE_USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true); 

  if (Firebase.ready()) {
      Firebase.RTDB.setString(&fbdo, "sensors/plantsense/debug_log", "JUST BOOTED (MATTER ENABLED)!");
  }
  
  // Setup SinricPro Device Callbacks
  SinricProSwitch& mySwitch = SinricPro[LAMP_DEVICE_ID];
  mySwitch.onPowerState(onPowerState);
  
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() {
  ArduinoOTA.handle();
  SinricPro.handle();

  if (!Matter.isDeviceCommissioned() && !printedCommissioning) {
    Serial.println("\n==================================================");
    Serial.println("📱 MATTER COMMISSIONING READY 📱");
    Serial.println("Matter Node is NOT commissioned yet.");
    Serial.println("To pair with Apple Home or Google Home:");
    Serial.printf("1. Manual pairing code: %s\n", Matter.getManualPairingCode().c_str());
    Serial.printf("2. QR code URL: %s\n", Matter.getOnboardingQRCodeUrl().c_str());
    Serial.println("Click the URL above, open the QR code on your computer, and scan it with your phone!");
    Serial.println("==================================================\n");
    printedCommissioning = true;
  }

  // Handle Matter Lamp State Sync
  bool currentMatterState = matterLamp.getOnOff();
  if (currentMatterState != lampState) {
      setLampState(currentMatterState);
      
      if (Firebase.ready()) {
          FirebaseJson fbJson;
          fbJson.set("lamp_state", currentMatterState ? "ON" : "OFF");
          Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &fbJson);
      }
      Serial.printf("Lamp toggled %s (via Matter)\r\n", currentMatterState ? "on" : "off");
  }

  // Handle Physical Button
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        bool newLampState = !getLampState();
        setLampState(newLampState);
        
        matterLamp.setOnOff(newLampState);
        SinricProSwitch& mySwitch = SinricPro[LAMP_DEVICE_ID];
        mySwitch.sendPowerStateEvent(newLampState);
        
        if (Firebase.ready()) {
            FirebaseJson fbJson;
            fbJson.set("lamp_state", newLampState ? "ON" : "OFF");
            Firebase.RTDB.updateNode(&fbdo, "sensors/plantsense", &fbJson);
        }
        Serial.printf("Lamp toggled %s (via Physical Button)\r\n", newLampState ? "on" : "off");
      }
    }
  }
  lastButtonState = reading;

  // Polling Firebase & Sensors
  if (Firebase.ready() && (millis() - sendDataPrevMillis > SEND_DATA_DELAY || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    if (Firebase.RTDB.getString(&fbdo, "sensors/plantsense/lamp_command")) {
      String command = fbdo.stringData();
      if ((command == "ON" && !lampState) || (command == "OFF" && lampState)) {
          bool newState = (command == "ON");
          setLampState(newState);
          matterLamp.setOnOff(newState);
          Serial.printf("Lamp turned %s (via Firebase Command)\r\n", newState ? "on" : "off");
      }
      Firebase.RTDB.deleteNode(&fbdo, "sensors/plantsense/lamp_command");
    }

    readAndSendPlantData(fbdo, getLampState());
  }
}
