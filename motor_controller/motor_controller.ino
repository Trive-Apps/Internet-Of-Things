#include <WiFi.h>
#include <WebServer.h>
#include "credential.h"

// Motor pins (L298N)
const int IN1 = 26;
const int IN2 = 27;
const int IN3 = 33;
const int IN4 = 32;
const int ENA = 14;
const int ENB = 13;

// PWM settings
const int pwmFrequency = 5000; // 5 KHz
const int pwmResolution = 8;   // 8-bit (0-255)

// Web Server Setting
WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

    // Set up the web server routes
  server.on("/", handleRoot);
  server.on("/forward", handleMoveForward);
  server.on("/backward", handleMoveBackward);
  server.on("/left", handleTurnLeft);
  server.on("/right", handleTurnRight);
  server.on("/stop", handleMotorStop);
  server.on("/setSpeed", handleSetMotorSpeed);

  // Start the web server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PWM initialization
  ledcAttach(ENA, pwmFrequency, pwmResolution);
  ledcAttach(ENB, pwmFrequency, pwmResolution);

  // Set the motorbike to stop
  handleMotorStop();
}

void loop() {
  server.handleClient();
}

// Function to handle root
void handleRoot() {
  String html ="<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>ESP32 Remote Control & Firestore Data</title> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\"> <link rel=\"stylesheet\" href=\"https://unpkg.com/leaflet";
  html +="1.7.1/dist/leaflet.css\" /> <style> #map { height: 300px; width: 100%; } .controller-btn { width: 100px; height: 100px; font-size: 24px; } </style> <script src=\"https://unpkg.com/leaflet";
  html +="1.7.1/dist/leaflet.js\"></script> <script> function convertUnixTimestamp(unix_timestamp) { const date = new Date(unix_timestamp * 1000); return date.toISOString().replace('T', ' ').substring(0, 19); } function initializeMap(lat, lon) { const map = L.map('map').setView([lat, lon], 13); L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', { attribution: '© OpenStreetMap contributors' }).addTo(map); L.marker([lat, lon]).addTo(map) .bindPopup(`Latitude: ${lat}, Longitude: ${lon}`) .openPopup(); } async function fetchFirestoreData() { const url = 'https://firestore.googleapis.com/v1/projects/trive-221ec/databases/(default)/documents/car'; try { const response = await fetch(url); const data = await response.json(); if (data.documents) { const sortedDocuments = data.documents.sort((a, b) => new Date(b.createTime) - new Date(a.createTime)); const latestDocument = sortedDocuments[0]; const fields = latestDocument.fields; document.getElementById('carBrand').innerText = fields.brand.stringValue; document.getElementById('carType').innerText = fields.type.stringValue; document.getElementById('temperature').innerText = fields.temperature.doubleValue + ' °C'; document.getElementById('power').innerText = fields.power.doubleValue + ' kW'; document.getElementById('voltage').innerText = fields.voltage.doubleValue + ' V'; document.getElementById('current').innerText = fields.current.doubleValue + ' A'; document.getElementById('chargerType').innerText = fields.chargerType.stringValue; document.getElementById('latitude').innerText = fields.latitude.doubleValue; document.getElementById('longitude').innerText = fields.longitude.doubleValue; const unixTimestamp = fields.timestamp.integerValue; const readableTime = convertUnixTimestamp(unixTimestamp); document.getElementById('timestamp').innerText = readableTime; initializeMap(fields.latitude.doubleValue, fields.longitude.doubleValue); } } catch (error) { console.error('Error fetching Firestore data:', error); } } window.onload = fetchFirestoreData; function sendCommand(command) { var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"/\" + command, true); xhttp.send(); } function setSpeed() { var speed = document.getElementById(\"speedSlider\").value; var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"/setSpeed?speedA=\" + speed + \"&speedB=\" + speed, true); xhttp.send(); } function updateSpeedLabel() { var slider = document.getElementById(\"speedSlider\"); var label = document.getElementById(\"speedLabel\"); label.innerText = slider.value; } </script> </head> <body style=\"height: 100vh; overflow: hidden;\"> <div class=\"container-fluid h-100\"> <div class=\"row h-100\"> <!-- Data mobil dan peta --> <div class=\"col overflow-auto\" style=\"max-height: 100%;\"> <div class=\"card mt-4\"> <div class=\"card-body\"> <h5 class=\"card-title\">Car Data</h5> <p><strong>Brand:</strong> <span id=\"carBrand\">Loading...</span></p> <p><strong>Type:</strong> <span id=\"carType\">Loading...</span></p> <p><strong>Temperature:</strong> <span id=\"temperature\">Loading...</span></p> <p><strong>Power:</strong> <span id=\"power\">Loading...</span></p> <p><strong>Voltage:</strong> <span id=\"voltage\">Loading...</span></p> <p><strong>Current:</strong> <span id=\"current\">Loading...</span></p> <p><strong>Charger Type:</strong> <span id=\"chargerType\">Loading...</span></p> <p><strong>Latitude:</strong> <span id=\"latitude\">Loading...</span></p> <p><strong>Longitude:</strong> <span id=\"longitude\">Loading...</span></p> <p><strong>Timestamp:</strong> <span id=\"timestamp\">Loading...</span></p> </div> </div> <div class=\"card mt-4\"> <div class=\"card-body\"> <h5 class=\"card-title\">Location Map</h5> <div id=\"map\" style=\"height: 300px;\">Loading map...</div> </div> </div> </div> <!-- Remote control buttons with 3 rows --> <div class=\"col overflow-auto\" style=\"max-height: 100%;\"> <div class=\"row text-center mt-4\"> <div class=\"col-12\"> <button class=\"btn btn-success controller-btn\" onclick=\"sendCommand('forward')\">Maju</button> </div> </div> <div class=\"row text-center mt-4\"> <div class=\"col-4\"> <button class=\"btn btn-warning controller-btn\" onclick=\"sendCommand('left')\">Kiri</button> </div> <div class=\"col-4\"> <button class=\"btn btn-danger controller-btn\" onclick=\"sendCommand('stop')\">Stop</button> </div> <div class=\"col-4\"> <button class=\"btn btn-warning controller-btn\" onclick=\"sendCommand('right')\">Kanan</button> </div> </div> <div class=\"row text-center mt-4\"> <div class=\"col-12\"> <button class=\"btn btn-success controller-btn\" onclick=\"sendCommand('backward')\">Mundur</button> </div> </div> <!-- Kontrol kecepatan menggunakan satu slider --> <div class=\"row text-center mt-4\"> <div class=\"col-md-12\"> <label for=\"speedSlider\">Kecepatan Motor: <span id=\"speedLabel\">120</span></label> <input type=\"range\" id=\"speedSlider\" class=\"form-control-range\" min=\"120\" max=\"255\" value=\"120\" oninput=\"updateSpeedLabel()\"> </div> </div> <div class=\"row text-center mt-4\"> <div class=\"col-md-12\"> <button class=\"btn btn-primary btn-lg\" onclick=\"setSpeed()\">Set Speed</button> </div> </div> </div> </div> </div> </body>";

  server.send(200, "text/html", html);
}

// Function to regulate motor speed simultaneously
void setMotorSpeed(int speedA, int speedB) {
  ledcWrite(ENA, speedA); // Set left motor speed
  ledcWrite(ENB, speedB); // Set right motor speed
}

// Function to regulate motor speed simultaneously
void handleSetMotorSpeed() {
  if (server.hasArg("speedA") && server.hasArg("speedB")) {
    int speedA = server.arg("speedA").toInt();
    int speedB = server.arg("speedB").toInt();
    setMotorSpeed(speedA, speedB);  // Panggil fungsi untuk set kecepatan motor
    Serial.printf("SpeedA: %d, SpeedB: %d\n", speedA, speedB);
    server.send(200, "text/plain", "Speed updated");
  } else {
    server.send(400, "text/plain", "Missing speed parameters");
  }
}


void handleMoveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  setMotorSpeed(190, 190);
  Serial.println("Moving forward");
  server.send(200, "text/plain", "Forward");
}

void handleMoveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  setMotorSpeed(190, 190);
  Serial.println("Moving backward");
  server.send(200, "text/plain", "Backward");
}

void handleTurnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  
  setMotorSpeed(120, 190);
  Serial.println("Turning left");
  server.send(200, "text/plain", "Turning left");
}

void handleTurnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorSpeed(190, 120);
  Serial.println("Turning right");
  server.send(200, "text/plain", "Turning right");
}

void handleMotorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  setMotorSpeed(0, 0);
  Serial.println("Motor stopped");
  server.send(200, "text/plain", "Motor stop");
}
