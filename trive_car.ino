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

// unsigned long interval = 120000;                            // 2 min interval to send data
unsigned long interval = 3000;                              // 500 millis interval to send data
unsigned long prev_millis = 0;


// =====================================================
// |        INA21 CURRENT SENSOR CONFIGURATION         |
// =====================================================

Adafruit_INA219 ina219;

float voltage_V, current_mA, power_mW, battery_percentage;
float calibrated_voltage_V, calibrated_current_A, calibrated_power_kW;

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

float calibrate_voltage(float value){
  const float target_voltage = 400.0;
  return (target_voltage / value) * value; 
}

float calibrate_current(float value){
  const float target_current = 2.5;
  return (target_current / (value / 1000)) * value;       // 1000 is value to convert mA to A
}

float calibrate_power(float value){
  return value / 1000000;                                 // 1000000 is value to convert mW to kW
}

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

  // Calibrating
  calibrated_voltage_V = calibrate_voltage(voltage_V);
  calibrated_current_A = calibrate_current(current_mA);
  calibrated_power_kW = calibrate_power(power_mW);
}

void display_data(){
  Serial.println("==========================================================================================");
  Serial.print("Voltage\t\t: "); Serial.print(voltage_V); Serial.println("V");
  Serial.print("Current\t\t: "); Serial.print(current_mA); Serial.println("mA");
  Serial.print("Power\t\t: "); Serial.print(power_mW); Serial.println("mW");
  Serial.print("C.Voltage\t: "); Serial.print(calibrated_voltage_V); Serial.println("V");
  Serial.print("C.Current\t: "); Serial.print(calibrated_current_A); Serial.println("A");
  Serial.print("C.Power\t\t: "); Serial.print(calibrated_power_kW, 6); Serial.println("kW");
  Serial.print("Temperature\t: "); Serial.print(temp_C); Serial.println("C");
  Serial.print("Battery\t\t: "); Serial.print(battery_percentage); Serial.println("%\n");
}

String get_json_data(){
  Serial.println("Creating JSON data...");

  // Create JSON data to send
  StaticJsonDocument<200> doc;
  doc["fields"]["voltage"]["doubleValue"] = voltage_V;
  doc["fields"]["current"]["doubleValue"] = current_mA;
  doc["fields"]["power"]["doubleValue"] = power_mW;
  doc["fields"]["calibrated_voltage"]["doubleValue"] = calibrated_voltage_V;
  doc["fields"]["calibrated_current"]["doubleValue"] = calibrated_current_A;
  doc["fields"]["calibrated_power"]["doubleValue"] = calibrated_power_kW;
  doc["fields"]["temperature"]["doubleValue"] = temp_C;
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

void display_banner(){
  Serial.println("                                                                                             ");  
  Serial.println("        -@@@@@@@@#.                                                                          ");
  Serial.println("        +@@#=-+%@@=                                                                          ");
  Serial.println("   +@@@@=         #@@@@                                                                      ");
  Serial.println("  .  +@@@@#    .@@@@@-  .                                                                    ");
  Serial.println(" #@@+   %@@@* @@@@+   %@@=    #@@@@@@@@@@@=.@@@@@@@@@@@# *@@  #@@@@@    @@@@@  %@@@@@@@@@@=  ");
  Serial.println(".@@@:@@%  @@@@@@* :@@+-@@%         %@+     .@@       +@% *@@   =@@@      %@%   %@*           ");
  Serial.println("-@@% =@@@+ *@@@  @@@@  @@@         %@+     .@@       *@% *@@    :@@@@@  @@+    %@@@@@@@@#    ");
  Serial.println(":@@@   %@@% =@  @@@-  .@@@         %@+     .@@@@@@@@@@@: *@@     :@@@=.@@+     %@@++++++:    ");
  Serial.println(" %@@-   +@@#   @@@.   #@@+         %@+     .@@     @@@   *@@       @#-@@-      %@*           ");
  Serial.println("  @@@=   %@@: #@@=   %@@%          %@+     .@@      -@@+ *@@        =@@        %@@@@@@@@@@=  ");
  Serial.println("   @@@@= +@@* @@@. %@@@+                                                                     ");
  Serial.println("    .@@@+-@@# @@@.%@@#        ========= Effective Driving for Sustainable Living ==========  ");
  Serial.println("       ..-@@# @@@ :                                                                          ");
  Serial.println("                                                                                             ");
}


// =====================================================
// |                   MAIN FUNCTION                   |
// =====================================================

void setup() {
  // Open serial communications
  Serial.begin(115200);
  
  Serial.println("==========================================================================================");
  display_banner();

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
  Serial.println("DHT22 sensor ready\n");
}

void loop() {
  unsigned long cur_millis = millis();

  // Mengeksekusi setiap 2 menit
  if (cur_millis - prev_millis >= interval) {
    prev_millis = cur_millis;

    read_data();
    display_data();
    // send_data();
  }
}