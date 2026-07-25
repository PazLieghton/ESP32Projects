#include <Tiny4kOLED.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ---- Pin definitions ----
// ATtiny85 hardware I2C (USI) uses FIXED pins: SDA = PB0, SCL = PB2.
// Because of that, CE_PIN had to move off PB2 -> PB3.
// IO_PIN and SCLK_PIN are unchanged from your original sketch.

const int IO_PIN   = 4;  // PB4 - DS1302 DAT (unchanged)
const int SCLK_PIN = 5;  // PB5 - DS1302 CLK (unchanged, reset pin repurposed)
const int CE_PIN   = 3;  // PB3 - DS1302 RST/CE (moved from 2, now used by I2C SCL)

ThreeWire myWire(IO_PIN, SCLK_PIN, CE_PIN);
RtcDS1302<ThreeWire> Rtc(myWire);

void printTwoDigits(uint8_t v) {
    if (v < 10) oled.print('0');
    oled.print(v);
}

void setup() {
    oled.begin();
    oled.setFont(FONT6X8);
    oled.clear();
    oled.on();

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.IsDateTimeValid()) {
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected()) {
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning()) {
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Rtc.SetDateTime(compiled);
    }
}

void loop() {
    RtcDateTime now = Rtc.GetDateTime();

    oled.clear();

    if (!now.IsValid()) {
        oled.setFont(FONT6X8);
        oled.setCursor(0, 0);
        oled.print(F("RTC Read Error!"));
    } else {
        // Date line: DD/MM/YYYY
        oled.setFont(FONT6X8);
        oled.setCursor(0, 0);
        printTwoDigits(now.Day());
        oled.print('/');
        printTwoDigits(now.Month());
        oled.print('/');
        oled.print(now.Year());

        // Time line: HH:MM:SS (bigger font)
        oled.setFont(FONT8X16);
        oled.setCursor(16, 2);
        printTwoDigits(now.Hour());
        oled.print(':');
        printTwoDigits(now.Minute());
        oled.print(':');
        printTwoDigits(now.Second());
    }

    delay(1000);
}