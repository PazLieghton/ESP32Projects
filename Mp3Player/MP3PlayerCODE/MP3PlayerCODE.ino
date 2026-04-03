#include "DFRobotDFPlayerMini.h"
#include "HardwareSerial.h"

HardwareSerial mySerial(2); // Use UART2
DFRobotDFPlayerMini player;

void setup() {
  Serial.begin(115200);

  // RX, TX
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Initializing DFPlayer...");

  if (!player.begin(mySerial)) {
    Serial.println("DFPlayer NOT detected!");
    while (true);
  }

  Serial.println("DFPlayer ready!");

  player.volume(30);  // 0–30

  // Play first file automatically
  player.play(1);
}

void loop() {
  if (player.available()) {
    uint8_t type = player.readType();
    int value = player.read();

    switch (type) {
      case DFPlayerPlayFinished:
        Serial.print("Finished playing: ");
        Serial.println(value);
        break;

      case DFPlayerError:
        Serial.print("DFPlayer Error: ");
        Serial.println(value);
        break;

      default:
        Serial.print("Event: ");
        Serial.print(type);
        Serial.print(" Value: ");
        Serial.println(value);
        break;
    }
  }
}