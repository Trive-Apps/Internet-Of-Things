#include "Wire.h"
#include "Adafruit_INA219.h"
#include "credential.h"
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


// =====================================================
// |               TIMING CONFIGURATION                |
// =====================================================

unsigned long arr_interval[] = {120000, 60000, 30000, 5000};  // 2 min, 1 min, 30 sec, 5 sec interval to send data
unsigned long interval = arr_interval[2];
unsigned long prev_millis = 0;


// =====================================================
// |                DATA CONFIGURATION                 |
// =====================================================

const bool arr_data_config[] = {
  false,   // Voltage
  false,   // Current
  false,   // Power
  true,   // Calibrated Voltage
  true,   // Calibrated Current
  true,   // Calibrated Power
  true,   // Temperature
  false,   // Battery Percentage
};


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
  const float calibrator_voltage = 36.03;
  return value * calibrator_voltage; 
}

float calibrate_current(float value){
  const float calibrator_current = 58.82;
  return (value / 1000) * calibrator_current;             // 1000 is value to convert mA to A
}

float calibrate_power(float value){
  // return value / 1000000;                                 // 1000000 is value to convert mW to kW
  return value / 3.5;                                       // ((value / 1000000) * 1000000) / 3.5
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
  Serial.println("=================================");
  Serial.println("| Parameter      | Value\t|");
  Serial.println("=================================");

  if(arr_data_config[0]) {Serial.print("| Voltage        | "); Serial.print(voltage_V, 2); Serial.println(" V\t|");}
  if(arr_data_config[1]) {Serial.print("| Current        | "); Serial.print(current_mA, 2); Serial.println(" mA\t|");}
  if(arr_data_config[2]) {Serial.print("| Power          | "); Serial.print(power_mW, 2); Serial.println(" mW\t|");}
  if(arr_data_config[3]) {Serial.print("| C.Voltage      | "); Serial.print(calibrated_voltage_V, 2); Serial.println(" V\t|");}
  if(arr_data_config[4]) {Serial.print("| C.Current      | "); Serial.print(calibrated_current_A, 6); Serial.println(" A\t|");}
  if(arr_data_config[5]) {Serial.print("| C.Power        | "); Serial.print(calibrated_power_kW, 6); Serial.println(arr_data_config[0] ? " kWh\t|" : " kWh |");}
  if(arr_data_config[6]) {Serial.print("| Temperature    | "); Serial.print(temp_C, 2); Serial.println(" C\t|");}
  if(arr_data_config[7]) {Serial.print("| Battery        | "); Serial.print(battery_percentage, 2); Serial.println(" %\t|");}
  
  Serial.println("=================================");
}


String get_json_data(){
  Serial.println("Creating JSON data...");

  // Create JSON data to send
  StaticJsonDocument<200> doc;

  if(arr_data_config[0]) doc["fields"]["voltage"]["doubleValue"] = voltage_V;
  if(arr_data_config[1]) doc["fields"]["current"]["doubleValue"] = current_mA;
  if(arr_data_config[2]) doc["fields"]["power"]["doubleValue"] = power_mW;
  if(arr_data_config[3]) doc["fields"]["calibrated_voltage"]["doubleValue"] = calibrated_voltage_V;
  if(arr_data_config[4]) doc["fields"]["calibrated_current"]["doubleValue"] = calibrated_current_A;
  if(arr_data_config[5]) doc["fields"]["calibrated_power"]["doubleValue"] = calibrated_power_kW;
  if(arr_data_config[6]) doc["fields"]["temperature"]["doubleValue"] = temp_C;
  if(arr_data_config[7]) doc["fields"]["percentage"]["doubleValue"] = battery_percentage;

  // Convert JSON to string
  String jsonData;
  serializeJson(doc, jsonData);

  // Show data in Serial
  Serial.println("JSON data created");

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
      Serial.println("Data sended to Firestore");

      String response = http.getString();                 // Respons from server
      Serial.println("Response: " + response);
    } else {
      Serial.println("Fail to send data");
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
  
  display_banner();

  // Connecting to WiFi
  WiFi.begin(SSID, PASSWORD);
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

  Serial.println("Car Started\n");
}

void loop() {
  unsigned long cur_millis = millis();

  // Mengeksekusi setiap 2 menit
  if (cur_millis - prev_millis >= interval) {
    prev_millis = cur_millis;

    read_data();
    display_data();
    send_data();
  }
}