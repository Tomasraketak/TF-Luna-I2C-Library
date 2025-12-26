#ifndef TFLuna_h
#define TFLuna_h

#include <Arduino.h>
#include <Wire.h>

class TFLuna {
  public:
    // Konstruktor - defaultně 0x10, defaultně nic nemění (ODR zůstává 100Hz)
    TFLuna(uint8_t address = 0x10);

    bool begin(int sdaPin, int sclPin);
    bool read();

    // Čtení naměřených hodnot
    uint16_t getDistance();       // cm
    uint16_t getSignalStrength(); // libovolná jednotka
    float getTemperature();       // °C

    // --- Nastavení senzoru ---
    
    // Změna frekvence (1-250 Hz). Pokud nezavoláš, je tam default 100 Hz.
    bool setFrameRate(uint16_t fps);
    
    // Změna I2C adresy (např. z 0x10 na 0x11)
    bool setI2CAddress(uint8_t newAddr);

    // Uloží aktuální nastavení (frekvenci i adresu) do trvalé paměti
    bool saveSettings();

    // Obnoví tovární nastavení (Adresa 0x10, 100Hz, UART mód atd.)
    bool factoryReset();

  private:
    uint8_t _address;
    uint16_t _dist;
    uint16_t _strength;
    float _temp;
    
    // Pomocné funkce pro zápis
    bool writeReg(uint8_t reg, uint16_t value);
    bool writeRegByte(uint8_t reg, uint8_t value);
};

#endif