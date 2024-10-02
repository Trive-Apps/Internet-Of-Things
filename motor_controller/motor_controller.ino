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
  String html ="<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>ESP32 Remote Control</title> <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css\"> <style> .control-btn { width: 100%; margin-bottom: 15px; } </style> </head> <body> <div class=\"container mt-5\"> <h1 class=\"text-center mb-4\">Trive Car Remote Control</h1> <!-- Kontrol Remote --> <div class=\"row text-center\"> <div class=\"col-12 col-md-6 offset-md-3\"> <button class=\"btn btn-success control-btn\" onclick=\"sendCommand('forward')\">Maju</button> </div> </div> <div class=\"row text-center\"> <div class=\"col-4\"> <button class=\"btn btn-warning control-btn\" onclick=\"sendCommand('left')\">Belok Kiri</button> </div> <div class=\"col-4\"> <button class=\"btn btn-danger control-btn\" onclick=\"sendCommand('stop')\">Stop</button> </div> <div class=\"col-4\"> <button class=\"btn btn-warning control-btn\" onclick=\"sendCommand('right')\">Belok Kanan</button> </div> </div> <div class=\"row text-center\"> <div class=\"col-12 col-md-6 offset-md-3\"> <button class=\"btn btn-success control-btn\" onclick=\"sendCommand('backward')\">Mundur</button> </div> </div> </div> <!-- Tempat untuk menampilkan data terbaru dari Firestore --> <div id=\"carData\" class=\"mb-4\"></div> <script> // Fungsi untuk mengirim command ke server ESP32 function sendCommand(command) { var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"/\" + command, true); xhttp.send(); } </script> </body> </html>";
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
