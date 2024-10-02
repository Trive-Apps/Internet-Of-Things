#include "Wire.h"
#include "Adafruit_INA219.h"
#include "credential.h"
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>


// =====================================================
// |                DATA CONFIGURATION                 |
// =====================================================

// The data to be sent
const bool arr_data_config[] = {
  false,   // Voltage
  false,   // Current
  false,   // Power
  true,   // Calibrated Voltage
  true,   // Calibrated Current
  true,   // Calibrated Power
  true,   // Persentage Calibrated Power
  true,   // Temperature
  false,   // Battery Percentage
  true,   // Latitude
  true,   // Longtitude
  true,   // Is Active?
  true,   // Timestamp?
};

// Other data to be sent
const String token = "SCI-030924";
const String type = "BMW I3";
const String brand = "BMW";
const String charger_type = "CCS2";

// State whether the car is at fault
bool is_active = false;

// Time interval for updating data
unsigned long arr_interval[] = {120000, 60000, 30000, 15000, 5000};  // 2 min, 1 min, 30 sec, or 5 sec interval to send data
unsigned long interval = arr_interval[3];
unsigned long prev_millis = 0;

// Iteration
int iteration = 1;

// =====================================================
// |            GPS LOCATION CONFIGURATION             |
// =====================================================

// const double latitude = -6.25192922646693;
// const double longitude = 106.84180629007942;
double latitude, longitude;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
const short rx_pin = 16, tx_pin = 17;
SoftwareSerial ss(rx_pin, tx_pin);


// =====================================================
// |        INA21 CURRENT SENSOR CONFIGURATION         |
// =====================================================

Adafruit_INA219 ina219;

float voltage_V, current_mA, power_mW, battery_percentage;
float calibrated_voltage_V, calibrated_current_A, calibrated_power_kW, persentage_calibrated_power;

const float full_voltage = 12.6;
const float zero_voltage = 9.0;


// =====================================================
// |      DHT22 TEMPERATURE SENSOR CONFIGURATION       |
// =====================================================

#define DHT22_PIN  25

DHT dht22(DHT22_PIN, DHT22);

float temp_C = 0;


// =====================================================
// |             NTP SERVER CONFIGURATION              |
// =====================================================

// NTP Server and time offset
// const long utc_offset_in_seconds = 28800; // 8 hours for Central Indonesian Time (WITA)
const long utc_offset_in_seconds = 25200; // 7 hours for Central Indonesian Time (WIB)
const char* ntp_server = "pool.ntp.org";

// Create an object for the NTP client
WiFiUDP ntp_UDP;
NTPClient timeClient(ntp_UDP, ntp_server, utc_offset_in_seconds);

unsigned long timestamp;  // Variable to store time in UNIX timestamp format


// =====================================================
// |                     FUNCTIONS                     |
// =====================================================

float calibrate_voltage(float value){
  return value * 35.44; 
}

float calibrate_current(float value){
  return (value / 1000) * 60.2;             // 1000 is value to convert mA to A
}

float calibrate_power(float value){
  // 1000000 is value to convert mW to kW
  // return (value / 1000000) * 677.42;
  return (value / 10000) * 677.42;
}

float get_power_persentage(float value){
  // percentagePower = (nilai sensor / 18.8 ) * 100%
  return (value / 18.8) * 100;
}

void read_data(){
  // Read bus Voltage, Current, and Power
  voltage_V = ina219.getBusVoltage_V();     // V
  current_mA = ina219.getCurrent_mA();      // mA
  power_mW = ina219.getPower_mW();          // mW

  // Calculates battery percentage
  // Battery Percentage = ((BV-9V)/(11.1V-9V))X100%
  battery_percentage = ((voltage_V - zero_voltage) / (full_voltage - zero_voltage)) * 100;

  // Read temperature in Celsius
  temp_C = dht22.readTemperature();

  // Calibrating
  calibrated_voltage_V = calibrate_voltage(voltage_V);    // V
  calibrated_current_A = calibrate_current(current_mA);   // A
  calibrated_power_kW = calibrate_power(power_mW);        // kW
  persentage_calibrated_power = get_power_persentage(calibrated_power_kW);    // %

  // Read GPS Data
  // Graha Sucofindo (-6.25165, 106.84190)
  latitude = gps.location.isValid() ? gps.location.lat() :  -6.25165;
  longitude = gps.location.isValid() ? gps.location.lng() : 106.84190;

  // Set is_active
  is_active = calibrated_current_A <= 0 ? false : true;

  // Update time from NTP server & gets the timestamp of the epoch time
  timeClient.update();
  timestamp = timeClient.getEpochTime();
}

void display_data(){
  Serial.println(String(iteration) + " x Iteration");
  print_readable_time(timestamp);

  Serial.println("=================================");
  Serial.println("| Parameter        | Value\t|");
  Serial.println("=================================");

  Serial.println("| Car Name         | " + brand + "\t|");
  Serial.println("| Car Type         | " + type + "\t|");
  Serial.println("| Car Token        | " + token + "\t|");
  Serial.println("| Charger Type     | " + charger_type + "\t|");

  if(arr_data_config[0]) {Serial.print("| P.Voltage        | "); Serial.print(voltage_V, 2); Serial.println(" V\t|");}
  if(arr_data_config[1]) {Serial.print("| P.Current        | "); Serial.print(current_mA, 2); Serial.println(" mA\t|");}
  if(arr_data_config[2]) {Serial.print("| P.Power          | "); Serial.print(power_mW, 2); Serial.println(arr_data_config[0] ? " mW\t|" : " mW |");}
  if(arr_data_config[3]) {Serial.print("| Voltage          | "); Serial.print(calibrated_voltage_V, 2); Serial.println(" V\t|");}
  if(arr_data_config[4]) {Serial.print("| Current          | "); Serial.print(calibrated_current_A, 2); Serial.println(" A\t|");}
  if(arr_data_config[5]) {Serial.print("| Power            | "); Serial.print(calibrated_power_kW, 2); Serial.println(" kW\t|");}
  if(arr_data_config[6]) {Serial.print("| Persentage Power | "); Serial.print(persentage_calibrated_power, 2); Serial.println(" %\t|");}
  if(arr_data_config[7]) {Serial.print("| Temperature      | "); Serial.print(temp_C, 2); Serial.println(" C\t|");}
  if(arr_data_config[8]) {Serial.print("| Battery          | "); Serial.print(battery_percentage, 2); Serial.println(" %\t|");}
  if(arr_data_config[9]) {Serial.print("| Latitude         | "); Serial.print(latitude); Serial.println("\t|");}
  if(arr_data_config[10]) {Serial.print("| Longitude        | "); Serial.print(longitude); Serial.println("\t|");}
  if(arr_data_config[11]) {Serial.print("| Is Active?       | "); Serial.println(is_active ? "True\t|" : "False\t|");}
  if(arr_data_config[12]) {Serial.print("| Timestamp        | "); Serial.print(timestamp); Serial.println("\t|");}
  
  Serial.println("=================================\n");
}

String get_json_data(){
  Serial.println("Creating JSON data...");

  // Create JSON data to send
  StaticJsonDocument<512> doc;

  doc["fields"]["brand"]["stringValue"] = brand;
  doc["fields"]["type"]["stringValue"] = type;
  doc["fields"]["token"]["stringValue"] = token;
  doc["fields"]["chargerType"]["stringValue"] = charger_type;

  if(arr_data_config[0]) doc["fields"]["pureVoltage"]["doubleValue"] = voltage_V;
  if(arr_data_config[1]) doc["fields"]["purCurrent"]["doubleValue"] = current_mA;
  if(arr_data_config[2]) doc["fields"]["purePower"]["doubleValue"] = power_mW;
  if(arr_data_config[3]) doc["fields"]["voltage"]["doubleValue"] = calibrated_voltage_V;
  if(arr_data_config[4]) doc["fields"]["current"]["doubleValue"] = calibrated_current_A;
  if(arr_data_config[5]) doc["fields"]["power"]["doubleValue"] = calibrated_power_kW;
  if(arr_data_config[6]) doc["fields"]["persentagePower"]["doubleValue"] = persentage_calibrated_power;
  if(arr_data_config[7]) doc["fields"]["temperature"]["doubleValue"] = temp_C;
  if(arr_data_config[8]) doc["fields"]["percentage"]["doubleValue"] = battery_percentage;
  if(arr_data_config[9]) doc["fields"]["latitude"]["doubleValue"] = latitude;
  if(arr_data_config[10]) doc["fields"]["longitude"]["doubleValue"] = longitude;
  if(arr_data_config[11]) doc["fields"]["isActive"]["booleanValue"] = is_active;
  if(arr_data_config[12]) doc["fields"]["timestamp"]["integerValue"] = timestamp;

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
      // Serial.println("Response: " + response);
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

void print_readable_time(unsigned long epoch_time) {
  // Konversi epoch time menjadi waktu yang mudah dibaca
  time_t raw_time = epoch_time;
  struct tm *time_info;
  time_info = localtime(&raw_time);

  // Format: HH:MM:SS DD/MM/YYYY
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%H:%M:%S %d/%m/%Y", time_info);

  // Cetak waktu yang telah diformat
  Serial.print("Waktu sekarang: ");
  Serial.println(buffer);
}

// =====================================================
// |                   MAIN FUNCTION                   |
// =====================================================

void setup(){
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

  // Initialixe NTP client
  timeClient.begin();
  Serial.println("NTP client ready");

  // Initialize Software Serial
  ss.begin(115200);
  Serial.println("Software serial ready");

  Serial.println("Car Started\n");
}

void loop(){
  unsigned long cur_millis = millis();

  if (cur_millis - prev_millis >= interval) {
    prev_millis = cur_millis;

    read_data();
    display_data();
    send_data();

    iteration++;
  }
}