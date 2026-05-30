// ============================================================
//  ESP32 WROOM — IR Remote + Smart Ultrasonic Exploration
//  Libraries needed: IRremote (by shirriff, z3t0, ArminJo)
// ============================================================

#include <IRremote.h>

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
//  TRIG → GPIO 22  |  ECHO → GPIO 4 (NOT GPIO 1)
// ============================================================
const int TRIG_PIN = 22;
const int ECHO_PIN = 4;

// How close (cm) before reacting — tightened from 20 to 12
const int OBSTACLE_CM = 12;

// ============================================================
//  SECTION 4 — SPEED & MODE STATE
// ============================================================
int  robotSpeed = 200;
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
//  SECTION 7 — ULTRASONIC DISTANCE FUNCTION
//
//  Key fixes:
//  - Timeout now returns 3 cm, NOT 999.
//    HC-SR04 blind zone is ~2-3 cm. If pulseIn times out it
//    almost always means something is RIGHT in front, not clear.
//    Returning 999 before was telling the robot "all clear"
//    when it was actually touching an obstacle.
//  - Takes 3 readings, returns the LOWEST (most conservative).
//    The median was hiding real close-range detections.
//  - delay(15) between pings lets the echo fully die out.
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
//  SECTION 8 — SCAN LEFT / CENTER / RIGHT
//
//  Briefly turns the robot to aim the sensor in each direction,
//  reads the distance, then returns to center. This means the
//  robot "looks" before deciding which way to go.
//
//  SCAN_TURN_MS — how long to turn to face each side (ms).
//  Keep this short: just enough to swing ~30–45 degrees.
//  Adjust if your robot turns too little or too much.
// ============================================================
const int SCAN_TURN_MS  = 180;  // ms to swing left or right for scan
const int SCAN_SPEED    = 160;  // slower speed for scanning turns

ScanResult scanSurroundings() {
  ScanResult result;

  // --- Read CENTER first (already facing forward) ---
  result.centerCM = getDistanceCM();
  Serial.print("Center: "); Serial.print(result.centerCM); Serial.println(" cm");

  // --- Swing LEFT, read, swing back to center ---
  turnLeft(SCAN_SPEED);
  delay(SCAN_TURN_MS);
  stopMotors();
  delay(80); // let robot settle before reading
  result.leftCM = getDistanceCM();
  Serial.print("Left:   "); Serial.print(result.leftCM); Serial.println(" cm");

  turnRight(SCAN_SPEED); // return to center
  delay(SCAN_TURN_MS);
  stopMotors();
  delay(80);

  // --- Swing RIGHT, read, swing back to center ---
  turnRight(SCAN_SPEED);
  delay(SCAN_TURN_MS);
  stopMotors();
  delay(80);
  result.rightCM = getDistanceCM();
  Serial.print("Right:  "); Serial.print(result.rightCM); Serial.println(" cm");

  turnLeft(SCAN_SPEED); // return to center
  delay(SCAN_TURN_MS);
  stopMotors();
  delay(80);

  return result;
}

// ============================================================
//  SECTION 9 — AUTONOMOUS EXPLORATION LOGIC
//
//  Strategy:
//   1. Drive forward while path is clear (tight threshold).
//   2. On obstacle: stop → back up just a little → SCAN all
//      three directions → turn toward the most open side.
//   3. If all directions are blocked, do a 180.
// ============================================================

// How far to turn toward the chosen direction after scanning (ms).
// Adjust TURN_MS if the robot doesn't turn enough / too much.
const int TURN_MS = 350;

void runAutoExploration() {
  long dist = getDistanceCM();

  if (dist > OBSTACLE_CM) {
    moveForward(robotSpeed);
    return; // nothing to do, keep going
  }

  // ── Obstacle detected ─────────────────────────────────────
  Serial.print("Obstacle at "); Serial.print(dist); Serial.println(" cm — scanning...");
  stopMotors();
  delay(100);

  // Back up just enough to have room to turn (~15cm worth)
  moveBackward(robotSpeed);
  delay(250);
  stopMotors();
  delay(100);

  // Scan all three directions
  ScanResult scan = scanSurroundings();

  // ── Decide which way to go ────────────────────────────────
  bool leftClear   = scan.leftCM   > OBSTACLE_CM;
  bool centerClear = scan.centerCM > OBSTACLE_CM;
  bool rightClear  = scan.rightCM  > OBSTACLE_CM;

  if (centerClear && scan.centerCM >= scan.leftCM && scan.centerCM >= scan.rightCM) {
    // Center is clear AND the best option — no turn needed
    Serial.println("Best path: CENTER — going straight");

  } else if (leftClear && (!rightClear || scan.leftCM >= scan.rightCM)) {
    // Left is clearer
    Serial.println("Best path: LEFT");
    turnLeft(robotSpeed);
    delay(TURN_MS);
    stopMotors();

  } else if (rightClear) {
    // Right is clearer
    Serial.println("Best path: RIGHT");
    turnRight(robotSpeed);
    delay(TURN_MS);
    stopMotors();

  } else {
    // All directions blocked — do a 180
    Serial.println("All blocked — doing 180");
    turnRight(robotSpeed);
    delay(TURN_MS * 2);
    stopMotors();
  }

  delay(100);
  // Resume forward on next loop iteration
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
        if (!autoMode) { Serial.println("Forward"); moveForward(robotSpeed); }
        break;

      case BTN_BACKWARD:
        if (!autoMode) { Serial.println("Backward"); moveBackward(robotSpeed); }
        break;

      case BTN_LEFT:
        if (!autoMode) { Serial.println("Left"); turnLeft(robotSpeed); }
        break;

      case BTN_RIGHT:
        if (!autoMode) { Serial.println("Right"); turnRight(robotSpeed); }
        break;

      case BTN_STOP:
        if (!autoMode) { Serial.println("Stop"); stopMotors(); }
        break;

      case BTN_SPEED_UP:
        robotSpeed += 25;
        if (robotSpeed > 255) robotSpeed = 255;
        Serial.print("Speed+: "); Serial.println(robotSpeed);
        if (motorsRunning()) { analogWrite(enableA, robotSpeed); analogWrite(enableB, robotSpeed); }
        break;

      case BTN_SPEED_DOWN:
        robotSpeed -= 25;
        if (robotSpeed < 0) robotSpeed = 0;
        Serial.print("Speed-: "); Serial.println(robotSpeed);
        if (motorsRunning()) { analogWrite(enableA, robotSpeed); analogWrite(enableB, robotSpeed); }
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
  handleIRRemote();

  if (autoMode) {
    runAutoExploration();
  }
}
