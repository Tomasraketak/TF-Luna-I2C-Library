// Základní čtení vzdálenosti, síly signálu a teploty z TF-Luna přes I2C
#include <TFLuna.h>

// Piny pro Raspberry Pi Pico / Pico 2 (I2C0 = Wire)
#define SDA_PIN 4
#define SCL_PIN 5

TFLuna luna; // výchozí adresa 0x10

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);

  while (!luna.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Senzor nenalezen - zkontroluj zapojení a pin 5 na GND (I2C režim).");
    delay(1000);
  }
  Serial.println("Senzor připraven");
}

void loop() {
  if (luna.read()) {
    if (luna.isValid()) {
      Serial.print("Vzdalenost: ");
      Serial.print(luna.getDistance());
      Serial.print(" cm");
    } else {
      Serial.print("Vzdalenost: --- (slabý nebo přesycený signál)");
    }
    Serial.print(" | Signal: ");
    Serial.print(luna.getSignalStrength());
    Serial.print(" | Teplota: ");
    Serial.print(luna.getTemperature());
    Serial.println(" C");
  } else {
    Serial.println("Chyba komunikace");
  }

  delay(20);
}
