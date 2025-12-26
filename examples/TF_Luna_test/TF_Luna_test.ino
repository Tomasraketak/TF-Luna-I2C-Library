#include "TFLuna.h"

// Adresa, na kterou to chceš změnit (např. 0x11)
#define NOVA_ADRESA 0x11

TFLuna luna; // Defaultně 0x10

void setup() {
  Serial.begin(115200);
  // Piny pro Pico
  luna.begin(4, 5);
  delay(1000);

  Serial.println("Měním adresu z 0x10 na 0x11...");

  if (luna.setI2CAddress(NOVA_ADRESA)) {
    Serial.println("Adresa změněna v RAM.");
    
    // DŮLEŽITÉ: Uložit, aby to zůstalo i po vypnutí
    luna.saveSettings();
    Serial.println("Nastavení uloženo. Prosím restartuj senzor.");
  } else {
    Serial.println("Chyba - senzor neodpovídá (možná už má novou adresu?)");
  }
}

void loop() {}