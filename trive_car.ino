#include "Wire.h"
#include "Adafruit_INA219.h"
#include "credential.h"
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


// =====================================================
// |                WIFI CONFIGURATION                 |
// =====================================================

const char* ssid = getSSID();
const char* password = getPassword();


// =====================================================
// |              FIREBASE CONFIGURATION               |
// =====================================================

const String FIRESTORE_URL = getFirestoreUrl();

// unsigned long interval = 120000;  // 2 min
unsigned long interval = 3000;  // 500 millis
unsigned long prev_millis = 0;


// =====================================================
// |        INA21 CURRENT SENSOR CONFIGURATION         |
// =====================================================

Adafruit_INA219 ina219;

float voltage_V, current_mA, power_mW, battery_percentage;
float 

const float full_voltage = 12.6;
const float zero_voltage = 9.0;




// =====================================================
// |      DHT22 TEMPERATURE SENSOR CONFIGURATION       |
// =====================================================

#define DHT22_PIN  25

DHT dht22(DHT22_PIN, DHT22);

float temp_C = 0;


// =====================================================
// |                     FUNCTIONS                     |
// =====================================================

void read_data(){
  // Read bus Voltage, Current, and Power
  voltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  // Calculates battery percentage
  // Battery Percentage = ((BV-9V)/(12.6V-9V))X100%
  battery_percentage = ((voltage_V - zero_voltage) / (full_voltage - zero_voltage)) * 100;

  // Read temperature in Celsius
  temp_C = dht22.readTemperature();
}

void print_data(){
  Serial.print("Temperature: "); Serial.print(temp_C, 1); Serial.print("C | ");
  Serial.print("Voltage: "); Serial.print(voltage_V, 1); Serial.print("V | ");
  Serial.print("Current: "); Serial.print(current_mA, 1); Serial.print("mA | ");
  Serial.print("Power: "); Serial.print(power_mW, 1); Serial.print("mW | ");
  Serial.print("Battery: "); Serial.print(battery_percentage, 1); Serial.println("%");
}

String get_json_data(){
  Serial.println("Creating JSON data...");

  // Create JSON data to send
  StaticJsonDocument<200> doc;
  doc["fields"]["temperature"]["doubleValue"] = temp_C;
  doc["fields"]["voltage"]["doubleValue"] = voltage_V;
  doc["fields"]["current"]["doubleValue"] = current_mA;
  doc["fields"]["power"]["doubleValue"] = power_mW;
  doc["fields"]["percentage"]["doubleValue"] = battery_percentage;

  // Convert JSON to string
  String jsonData;
  serializeJson(doc, jsonData);

  // Show data in Serial
  Serial.println("JSON data created");
  Serial.println(jsonData);

  return jsonData;
}

void send_data(){
  Serial.println("Sending data to Firestore...");

  String jsonData = get_json_data();

  // Send data to Firestore via REST API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(FIRESTORE_URL);                            // URL endpoint
    http.addHeader("Content-Type", "application/json");   // JSON Header
    
    short httpResponseCode = http.POST(jsonData);         // Send data with POST method

    if (httpResponseCode > 0) {
      String response = http.getString();                 // Respons from server
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error sending POST: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
  
}


// =====================================================
// |                   MAIN FUNCTION                   |
// =====================================================

void setup() {
  // Open serial communications
  Serial.begin(115200);

  // Connecting to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize the INA219 sensor
  ina219.begin();
  Serial.println("INA219 sensor ready");

  // Initialize the DHT22 sensor
  dht22.begin();
  Serial.println("DHT22 sensor ready");
}

void loop() {
  unsigned long cur_millis = millis();

  // Mengeksekusi setiap 2 menit
  if (cur_millis - prev_millis >= interval) {
    prev_millis = cur_millis;

    read_data();
    print_data();
    // send_data();
  }
}