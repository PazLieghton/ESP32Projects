/*********
  Adapted for ESP32 + L298N (Motor B: IN3=26, IN4=27, ENB=14)
  Speed control via PWM on enable pin.
*********/

// Motor B pins (L298N)
int in3Pin = 26;   // IN3
int in4Pin = 27;   // IN4
int enableBPin = 14;  // ENB (PWM capable)

// PWM settings
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;   // starting duty cycle (0-255)

void setup() {
  // Set direction pins as outputs
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(enableBPin, OUTPUT);

  // Attach PWM channel to enable pin
  ledcAttachChannel(enableBPin, freq, resolution, pwmChannel);

  Serial.begin(115200);
  Serial.println("Testing DC Motor (Motor B)");
}

void loop() {
  // --- Forward at full speed ---
  Serial.println("Moving Forward");
  digitalWrite(in3Pin, HIGH);
  digitalWrite(in4Pin, LOW);
  ledcWrite(enableBPin, 255);   // full speed
  delay(2000);

  // --- Stop ---
  Serial.println("Motor stopped");
  digitalWrite(in3Pin, LOW);
  digitalWrite(in4Pin, LOW);
  delay(1000);

  // --- Backward at full speed ---
  Serial.println("Moving Backward");
  digitalWrite(in3Pin, LOW);
  digitalWrite(in4Pin, HIGH);
  ledcWrite(enableBPin, 255);
  delay(2000);

  // --- Stop ---
  Serial.println("Motor stopped");
  digitalWrite(in3Pin, LOW);
  digitalWrite(in4Pin, LOW);
  delay(1000);

  // --- Forward with increasing speed (ramp up) ---
  Serial.println("Ramping speed forward");
  digitalWrite(in3Pin, HIGH);
  digitalWrite(in4Pin, LOW);
  
  dutyCycle = 0;
  while (dutyCycle <= 255) {
    ledcWrite(enableBPin, dutyCycle);
    Serial.print("Forward duty cycle: ");
    Serial.println(dutyCycle);
    dutyCycle = dutyCycle + 5;
    delay(500);
  }

  dutyCycle = 200;   // reset for next loop iteration
}