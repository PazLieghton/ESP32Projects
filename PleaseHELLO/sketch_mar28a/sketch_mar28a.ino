int rollCount = 0;
const int totalRolls = 1500;
float totalEarned = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  randomSeed(analogRead(0));

  while (rollCount < totalRolls) {
    int roll = random(1, 7);  // Simulate dice (1-6)
    float earnings = calculateEarnings(roll);
    totalEarned += earnings;
    rollCount++;

    float expectedValue = totalEarned / rollCount;

    // Print expected value to Serial Plotter
    Serial.println(expectedValue); // Only one variable so it graphs nicely

    delay(10);  // Plotter-friendly delay
  }
}

float calculateEarnings(int dice) {
  switch (dice) {
    case 1:
      return 100;
    case 2: {
      int draws = 0;
      while (true) {
        draws++;
        int ball = random(1, 16); // 1–10 red, 11–15 blue
        if (ball > 10) break;
      }
      return 10.0 * draws;
    }
    case 3: {
      int draws = 0;
      while (draws < 4) {
        draws++;
        int ball = random(1, 16);
        if (ball > 10) break;
      }
      return min(10.0 * draws, 40.0);
    }
    case 4: {
      int reds = 0;
      bool used[15] = { false };
      for (int i = 0; i < 6; i++) {
        int pick;
        do {
          pick = random(0, 15);
        } while (used[pick]);
        used[pick] = true;
        if (pick < 10) reds++;
      }
      return 10.0 * reds;
    }
    case 5: {
      int reds = 0;
      for (int i = 0; i < 6; i++) {
        int ball = random(1, 16);
        if (ball <= 10) reds++;
      }
      return 10.0 * reds;
    }
    case 6: {
      int reds = 0;
      for (int i = 0; i < 6; i++) {
        int ball = random(1, 16);
        if (ball <= 10) reds++;
      }
      return max(10.0 * reds, 20.0);
    }
  }
  return 0;  // Fallback (shouldn't happen)
}

void loop() {
  // Do nothing
}
