#include <IRremote.h>

// ----- Motor Pins -----
int enableA = 32;
int motorA1 = 33;
int motorA2 = 25;

int enableB = 26;
int motorB3 = 27;
int motorB4 = 14;

// ----- IR Receiver Pin -----
const int IR_RECEIVE_PIN = 36;

// ----- Default Speed (0-255) -----
int robotSpeed = 200;

// ------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(500);

  // ----- Motor pins -----
  pinMode(enableA, OUTPUT);
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(enableB, OUTPUT);
  pinMode(motorB3, OUTPUT);
  pinMode(motorB4, OUTPUT);

  // ----- IR Receiver -----
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("IR Receiver ready. Press buttons on your Car MP3 remote.");
}

// ----- Movement Functions -----
void moveForward(int speed) {
  digitalWrite(motorA1, HIGH);
  digitalWrite(motorA2, LOW);
  analogWrite(enableA, speed);

  digitalWrite(motorB3, HIGH);
  digitalWrite(motorB4, LOW);
  analogWrite(enableB, speed);
}

void moveBackward(int speed) {
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, HIGH);
  analogWrite(enableA, speed);

  digitalWrite(motorB3, LOW);
  digitalWrite(motorB4, HIGH);
  analogWrite(enableB, speed);
}

void turnLeft(int speed) {
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, HIGH);
  analogWrite(enableA, speed);

  digitalWrite(motorB3, HIGH);
  digitalWrite(motorB4, LOW);
  analogWrite(enableB, speed);
}

void turnRight(int speed) {
  digitalWrite(motorA1, HIGH);
  digitalWrite(motorA2, LOW);
  analogWrite(enableA, speed);

  digitalWrite(motorB3, LOW);
  digitalWrite(motorB4, HIGH);
  analogWrite(enableB, speed);
}

void stopMotors() {
  analogWrite(enableA, 0);
  analogWrite(enableB, 0);
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB3, LOW);
  digitalWrite(motorB4, LOW);
}

// ------------------------------------------------------------------
void loop() {
  if (IrReceiver.decode()) {
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      Serial.print("Received IR Command: 0x");
      Serial.println(IrReceiver.decodedIRData.command, HEX);

      switch (IrReceiver.decodedIRData.command) {
        case 0x8:   // Forward
          Serial.println("Moving Forward");
          moveForward(robotSpeed);
          break;

        case 0x5A:  // Backward
          Serial.println("Moving Backward");
          moveBackward(robotSpeed);
          break;

        case 0x18:  // Left
          Serial.println("Turning Left");
          turnLeft(robotSpeed);
          break;

        case 0x52:  // Right
          Serial.println("Turning Right");
          turnRight(robotSpeed);
          break;

        case 0x1C:  // Stop
          Serial.println("Stopping");
          stopMotors();
          break;

        case 0x15:  // Increase Speed
          robotSpeed += 25;
          if (robotSpeed > 255) robotSpeed = 255;
          Serial.print("Speed Increased: ");
          Serial.println(robotSpeed);
          // If motors are running, update their speed immediately
          if (digitalRead(motorA1) || digitalRead(motorA2) || digitalRead(motorB3) || digitalRead(motorB4)) {
            analogWrite(enableA, robotSpeed);
            analogWrite(enableB, robotSpeed);
          }
          break;

        case 0x7:   // Decrease Speed
          robotSpeed -= 25;
          if (robotSpeed < 0) robotSpeed = 0;
          Serial.print("Speed Decreased: ");
          Serial.println(robotSpeed);
          if (digitalRead(motorA1) || digitalRead(motorA2) || digitalRead(motorB3) || digitalRead(motorB4)) {
            analogWrite(enableA, robotSpeed);
            analogWrite(enableB, robotSpeed);
          }
          break;

        default:
          Serial.println("Unmapped button pressed.");
          break;
      }
    }
    IrReceiver.resume();
  }
}