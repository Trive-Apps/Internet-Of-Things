#include "BluetoothSerial.h"

// BluetoothSerial object declaration
BluetoothSerial SerialBT;

// Motor pins (L298N)
const int IN1 = 14;
const int IN2 = 12;
const int IN3 = 13;
const int IN4 = 15;
const int ENA = 3;
const int ENB = 1;

// PWM settings
const int pwmFrequency = 5000; // 5 KHz
const int pwmResolution = 8;   // 8-bit (0-255)

void setup() {
  Serial.begin(115200);

  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Initialize Bluetooth with name "TRIVE_CAR"
  SerialBT.begin("TRIVE_CAR");
  Serial.println("Bluetooth Started! Waiting for commands...");

  // PWM initialization
  ledcAttach(ENA, pwmFrequency, pwmResolution);
  ledcAttach(ENB, pwmFrequency, pwmResolution);

  // Set the motorbike to stop
  stopMotor();
}

void loop() {
  if (SerialBT.available()) {
    char command = SerialBT.read();

    Serial.println(command);

    // Ignore newline or carriage return characters
    if (command == '\n' || command == '\r') {
      return;
    }

    // Control motorbikes based on commands
    switch (command) {
      case 'F':
        moveForward();
        break;
      case 'B':
        moveBackward();
        break;
      case 'L':
        turnLeft();
        break;
      case 'R':
        turnRight();
        break;
      case 'S':
        stopMotor();
        break;
      case '1':  // Low motor speed
        setMotorSpeed(120, 120);
        break;
      case '2':  // Medium motor speed
        setMotorSpeed(190, 190);
        break;
      case '3':  // High motor speed
        setMotorSpeed(255, 255);
        break;
      default:
        Serial.println("Invalid command");
        break;
    }
  }
}

// Function to regulate motor speed simultaneously
void setMotorSpeed(int speedA, int speedB) {
  ledcWrite(ENA, speedA); // Set left motor speed
  ledcWrite(ENB, speedB); // Set right motor speed
}

void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  setMotorSpeed(190, 190);
  Serial.println("Moving forward");
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  setMotorSpeed(190, 190);
  Serial.println("Moving backward");
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  
  setMotorSpeed(120, 190);
  Serial.println("Turning left");
}

void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  setMotorSpeed(190, 120);
  Serial.println("Turning right");
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  setMotorSpeed(0, 0);
  Serial.println("Motor stopped");
}
