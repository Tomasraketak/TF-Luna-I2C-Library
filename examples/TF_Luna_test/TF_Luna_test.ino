// Změna I2C adresy senzoru TF-Luna (např. kvůli více senzorům na jedné sběrnici)
#include <TFLuna.h>

// Adresa, na kterou to chceš změnit (0x08 - 0x77)
#define NOVA_ADRESA 0x11

// Piny pro Pico (I2C0 = Wire)
#define SDA_PIN 4
#define SCL_PIN 5

TFLuna luna; // Defaultně 0x10

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!luna.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Chyba - senzor na adrese 0x10 neodpovídá (možná už má novou adresu?)");
    return;
  }

  Serial.println("Měním adresu z 0x10 na 0x11...");

  if (!luna.setI2CAddress(NOVA_ADRESA)) {
    Serial.println("Chyba - adresu se nepodařilo změnit (neplatná nebo obsazená adresa?)");
    return;
  }
  Serial.println("Adresa změněna v RAM.");

  // DŮLEŽITÉ: Uložit, aby to zůstalo i po vypnutí
  if (!luna.saveSettings()) {
    Serial.println("Chyba - nastavení se nepodařilo uložit.");
    return;
  }
  Serial.println("Nastavení uloženo.");

  // Restart senzoru a ověření, že odpovídá na nové adrese
  if (luna.reboot() && luna.getAddress() == NOVA_ADRESA) {
    Serial.print("Hotovo, senzor odpovídá na adrese 0x");
    Serial.println(luna.getAddress(), HEX);
  } else {
    Serial.println("Senzor po restartu neodpovídá na nové adrese. Zkus ho odpojit a znovu zapojit.");
  }
}

void loop() {}
