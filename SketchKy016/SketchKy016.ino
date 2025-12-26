#define RED_PIN     14
#define GREEN_PIN   12
#define BLUE_PIN    13
#define ONBOARD_LED 2
#define POT_PIN     32

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
}

// Convert hue (0–255) to RGB
void hueToRGB(int hue, int &r, int &g, int &b) {
  int region = hue / 43;            // 0–5
  int remainder = (hue - (region * 43)) * 6;

  int p = 0;
  int q = 255 - remainder;
  int t = remainder;

  switch (region) {
    case 0: r = 255; g = t;   b = p;   break; // Red → Yellow
    case 1: r = q;   g = 255; b = p;   break; // Yellow → Green
    case 2: r = p;   g = 255; b = t;   break; // Green → Cyan
    case 3: r = p;   g = q;   b = 255; break; // Cyan → Blue
    case 4: r = t;   g = p;   b = 255; break; // Blue → Magenta
    default:r = 255; g = p;   b = q;   break; // Magenta → Red
  }
}

void loop() {
  int potValue = analogRead(POT_PIN);          // 0–4095
  int hue = map(potValue, 0, 4095, 0, 255);

  int r, g, b;
  hueToRGB(hue, r, g, b);

  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);

  // Blink onboard LED
  digitalWrite(ONBOARD_LED, HIGH);
  delay(100);
  digitalWrite(ONBOARD_LED, LOW);
  delay(100);
}
