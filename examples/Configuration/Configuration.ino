// Výpis informací o senzoru a nastavení frekvence měření
#include <TFLuna.h>

#define SDA_PIN 4
#define SCL_PIN 5

TFLuna luna;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);

  if (!luna.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Senzor nenalezen.");
    while (true) delay(1000);
  }

  uint8_t major, minor, rev;
  if (luna.getFirmwareVersion(major, minor, rev)) {
    Serial.print("Firmware: ");
    Serial.print(major); Serial.print('.');
    Serial.print(minor); Serial.print('.');
    Serial.println(rev);
  }

  char sn[15];
  if (luna.getSerialNumber(sn, sizeof(sn))) {
    Serial.print("Seriove cislo: ");
    Serial.println(sn);
  }

  Serial.print("Puvodni frame rate: ");
  Serial.print(luna.getFrameRate());
  Serial.println(" Hz");

  // Nastavení na 250 Hz (jen do restartu - pro trvalou změnu zavolej saveSettings())
  if (luna.setFrameRate(250)) {
    Serial.print("Novy frame rate: ");
    Serial.print(luna.getFrameRate());
    Serial.println(" Hz");
  }
}

void loop() {}
