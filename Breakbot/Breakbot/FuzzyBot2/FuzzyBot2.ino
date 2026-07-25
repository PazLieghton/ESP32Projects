// ============================================================
//  ESP32 WROOM — IR Remote + Smart Ultrasonic Exploration
//  + KY-031 Knock Sensor (Bump Stop) + Servo Scanning
//  Libraries needed: 
//  1. IRremote (by shirriff, z3t0, ArminJo)
//  2. ESP32Servo (by Kevin Harrington)
// ============================================================

#include <IRremote.h>
#include <ESP32Servo.h> 

// Struct must be declared before any function that uses it as a return type
struct ScanResult {
  long leftCM;
  long centerCM;
  long rightCM;
};

// ============================================================
//  SECTION 1 — MOTOR PINS
// ============================================================
int enableA = 32;
int motorA1  = 33;
int motorA2  = 25;
int enableB  = 26;
int motorB3  = 27;
int motorB4  = 14;

// ============================================================
//  SECTION 2 — IR RECEIVER PIN
// ============================================================
const int IR_RECEIVE_PIN = 36;

// ============================================================
//  SECTION 3 — ULTRASONIC SENSOR PINS (HC-SR04)
//  TRIG → GPIO 22  |  ECHO → GPIO 4
// ============================================================
const int TRIG_PIN = 22;
const int ECHO_PIN = 4;

// ============================================================
//  SECTION 3b — KY-031 KNOCK / BUMP SENSOR (GPIO 2)
// ============================================================
#define KNOCK_PIN   2        // KY-031 signal pin
bool bumped = false;         // true = impact just detected (only used in auto mode)
unsigned long bumpClearTime = 0;   // when we can clear the flag

int bumpCounter=0;
const int BUMP_THRESHOLD=5;

// How close (cm) before reacting — ultrasonic threshold
const int OBSTACLE_CM = 12;

bool ignoreBumpSensor=false;
// ============================================================
//  SECTION 3c — SERVO MOTOR SETTINGS (GPIO 15)
// ============================================================
const int SERVO_PIN = 15;
Servo ultraServo;

// Easily adjustable parameters for angles and timing
const int ANGLE_CENTER = 90;   // Straight ahead
const int ANGLE_LEFT   = 160;  // Looking Left
const int ANGLE_RIGHT  = 20;   // Looking Right
const int SERVO_DELAY  = 1200;  // Time (ms) allowed for the physical servo to turn


// ============================================================
//  SECTION 4 — SPEED & MODE STATE
// ============================================================
int  robotSpeed = 175;
bool autoMode   = false;

// IR button hex codes (Car MP3 remote)
#define BTN_FORWARD      0x8
#define BTN_BACKWARD     0x5A
#define BTN_LEFT         0x18
#define BTN_RIGHT        0x52
#define BTN_STOP         0x1C
#define BTN_SPEED_UP     0x15
#define BTN_SPEED_DOWN   0x7
#define BTN_AUTO_TOGGLE  0x45  // Change to whichever button you want

// ============================================================
//  SECTION 5 — SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  delay(500);

  pinMode(enableA, OUTPUT);
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(enableB, OUTPUT);
  pinMode(motorB3, OUTPUT);
  pinMode(motorB4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(KNOCK_PIN, INPUT);   // KY-031 gives HIGH on knock

  // Initialize Servo
  ultraServo.attach(SERVO_PIN);
  setSensorAngle(ANGLE_CENTER); // Look straight ahead initially

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  randomSeed(analogRead(34));

  Serial.println("Robot ready. Press AUTO button (0x45) to explore.");
}

// ============================================================
//  SECTION 6 — MOTOR CONTROL FUNCTIONS
// ============================================================
void moveForward(int speed) {
  digitalWrite(motorA1, HIGH); digitalWrite(motorA2, LOW);  analogWrite(enableA, speed);
  digitalWrite(motorB3, HIGH); digitalWrite(motorB4, LOW);  analogWrite(enableB, speed);
}

void moveBackward(int speed) {
  digitalWrite(motorA1, LOW);  digitalWrite(motorA2, HIGH); analogWrite(enableA, speed);
  digitalWrite(motorB3, LOW);  digitalWrite(motorB4, HIGH); analogWrite(enableB, speed);
}

void turnLeft(int speed) {
  digitalWrite(motorA1, LOW);  digitalWrite(motorA2, HIGH); analogWrite(enableA, speed);
  digitalWrite(motorB3, HIGH); digitalWrite(motorB4, LOW);  analogWrite(enableB, speed);
}

void turnRight(int speed) {
  digitalWrite(motorA1, HIGH); digitalWrite(motorA2, LOW);  analogWrite(enableA, speed);
  digitalWrite(motorB3, LOW);  digitalWrite(motorB4, HIGH); analogWrite(enableB, speed);
}

void stopMotors() {
  analogWrite(enableA, 0); analogWrite(enableB, 0);
  digitalWrite(motorA1, LOW); digitalWrite(motorA2, LOW);
  digitalWrite(motorB3, LOW); digitalWrite(motorB4, LOW);
}

bool motorsRunning() {
  return digitalRead(motorA1) || digitalRead(motorA2) ||
         digitalRead(motorB3) || digitalRead(motorB4);
}

// ============================================================
//  SECTION 6b — ISOLATED SERVO CONTROL FUNCTION
// ============================================================
void setSensorAngle(int angle) {
  ultraServo.write(angle);
  delay(SERVO_DELAY); // Give the servo gear time to physically reach the angle
}

// ============================================================
//  SECTION 7 — ULTRASONIC DISTANCE FUNCTION
// ============================================================
long getDistanceCM() {
  long readings[3];

  for (int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long d = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms ~ 340cm max
    if (d == 0) {
      readings[i] = 3; // timeout = blind zone = treat as obstacle right there
    } else {
      readings[i] = d / 58;
      if (readings[i] < 3) readings[i] = 3; // clamp to sensor minimum
    }
    delay(15); // let echo fully die before next ping
  }

  // Return the MINIMUM of the 3 readings (safest choice)
  long minVal = readings[0];
  if (readings[1] < minVal) minVal = readings[1];
  if (readings[2] < minVal) minVal = readings[2];
  return minVal;
}

// ============================================================
//  SECTION 8 — SCAN LEFT / CENTER / RIGHT (UPDATED TO USE SERVO)
// ============================================================
ScanResult scanSurroundings() {
  ScanResult result;

  // --- Read CENTER first ---
  setSensorAngle(ANGLE_CENTER);
  result.centerCM = getDistanceCM();
  Serial.print("Center: "); Serial.print(result.centerCM); Serial.println(" cm");

  // --- Turn Servo LEFT and read ---
  setSensorAngle(ANGLE_LEFT);
  result.leftCM = getDistanceCM();
  Serial.print("Left:   "); Serial.print(result.leftCM); Serial.println(" cm");

  // --- Turn Servo RIGHT and read ---
  setSensorAngle(ANGLE_RIGHT);
  result.rightCM = getDistanceCM();
  Serial.print("Right:  "); Serial.print(result.rightCM); Serial.println(" cm");

  // --- Reset Servo back to CENTER ---
  setSensorAngle(ANGLE_CENTER);

  return result;
}

// ============================================================
//  SECTION 9 — AUTONOMOUS EXPLORATION LOGIC
// ============================================================
const int TURN_MS = 500;  // how long to turn toward chosen direction (ms)

void handleObstacleScanAndTurn() {
  // Back up slightly to have room to turn
  moveBackward(robotSpeed);
  delay(250);
  stopMotors();
  delay(100);

  ScanResult scan = scanSurroundings();

  bool leftClear   = scan.leftCM   > OBSTACLE_CM;
  bool centerClear = scan.centerCM > OBSTACLE_CM;
  bool rightClear  = scan.rightCM  > OBSTACLE_CM;

  if (centerClear && scan.centerCM >= scan.leftCM && scan.centerCM >= scan.rightCM) {
    Serial.println("Best path: CENTER — going straight");
  } else if (leftClear && (!rightClear || scan.leftCM >= scan.rightCM)) {
    Serial.println("Best path: LEFT");
    ignoreBumpSensor=true;
    turnLeft(robotSpeed);
    delay(TURN_MS);
    stopMotors();
    ignoreBumpSensor = false;
  } else if (rightClear) {
    Serial.println("Best path: RIGHT");
    ignoreBumpSensor=true;
    turnRight(robotSpeed);
    delay(TURN_MS);
    stopMotors();
    ignoreBumpSensor = false;
  } else {
    Serial.println("All blocked — doing 180");
    ignoreBumpSensor=true;
    turnRight(robotSpeed);
    delay(TURN_MS * 2);
    stopMotors();
    ignoreBumpSensor = false;
  }
  delay(100);
}

void runAutoExploration() {
  // If a physical bump was detected, pause for 1 second, then handle as obstacle
  if (bumped) {
    Serial.println(">>> Auto mode: BUMP – pausing 1 sec <<<");
    stopMotors();
    delay(200);                 // 1 second pause to see it detected something
    handleObstacleScanAndTurn();
    bumped = false;              // cleared because we've reacted
    return;
  }

  long dist = getDistanceCM();

  if (dist > OBSTACLE_CM) {
    moveForward(robotSpeed);
    return;
  }

  // Obstacle from ultrasonic
  Serial.print("Obstacle at "); Serial.print(dist); Serial.println(" cm — scanning...");
  stopMotors();
  delay(100);
  handleObstacleScanAndTurn();
}

// ============================================================
//  SECTION 10 — IR REMOTE HANDLER
// ============================================================
void handleIRRemote() {
  if (!IrReceiver.decode()) return;

  if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
    uint8_t cmd = IrReceiver.decodedIRData.command;
    Serial.print("IR Command: 0x"); Serial.println(cmd, HEX);

    switch (cmd) {

      case BTN_AUTO_TOGGLE:
        autoMode = !autoMode;
        if (autoMode) {
          Serial.println(">>> AUTO EXPLORATION ON <<<");
        } else {
          Serial.println(">>> MANUAL MODE <<<");
          stopMotors();
        }
        break;

      case BTN_FORWARD:
        if (!autoMode) {
          Serial.println("Forward");
          moveForward(robotSpeed);
        }
        break;

      case BTN_BACKWARD:
        if (!autoMode) {
          Serial.println("Backward");
          moveBackward(robotSpeed);
        }
        break;

      case BTN_LEFT:
        if (!autoMode) {
          Serial.println("Left");
          turnLeft(robotSpeed);
        }
        break;

      case BTN_RIGHT:
        if (!autoMode) {
          Serial.println("Right");
          turnRight(robotSpeed);
        }
        break;

      case BTN_STOP:
        if (!autoMode) {
          Serial.println("Stop");
          stopMotors();
        }
        break;

      case BTN_SPEED_UP:
        robotSpeed += 25;
        if (robotSpeed > 255) robotSpeed = 255;
        Serial.print("Speed+: "); Serial.println(robotSpeed);
        if (motorsRunning()) {
          analogWrite(enableA, robotSpeed);
          analogWrite(enableB, robotSpeed);
        }
        break;

      case BTN_SPEED_DOWN:
        robotSpeed -= 25;
        if (robotSpeed < 0) robotSpeed = 0;
        Serial.print("Speed-: "); Serial.println(robotSpeed);
        if (motorsRunning()) {
          analogWrite(enableA, robotSpeed);
          analogWrite(enableB, robotSpeed);
        }
        break;

      default:
        Serial.println("Unmapped button.");
        break;
    }
  }

  IrReceiver.resume();
}

// ============================================================
//  SECTION 11 — MAIN LOOP
// ============================================================
void loop() {

  // Normal remote handling
  handleIRRemote();

  // Autonomous mode
  if (autoMode) {

    // Require several consecutive bump detections
    if (digitalRead(KNOCK_PIN) == HIGH) {
      Serial.println("KNOCK HIGH");
      bumpCounter++;

      if (bumpCounter >= BUMP_THRESHOLD) {

        if (!bumped) {
          Serial.println(">>> BUMP CONFIRMED <<<");
          bumped = true;
        }

        bumpCounter = 0;
      }

    } else {

      bumpCounter = 0;

    }

    runAutoExploration();
  }
}