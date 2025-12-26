#include "TFLuna.h"

TFLuna::TFLuna(uint8_t address) {
  _address = address;
  _dist = 0;
  _strength = 0;
  _temp = 0.0;
}

bool TFLuna::begin(int sdaPin, int sclPin) {
  Wire.setSDA(sdaPin);
  Wire.setSCL(sclPin);
  Wire.begin();
  return true; 
}

bool TFLuna::read() {
  Wire.beginTransmission(_address);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return false;

  Wire.requestFrom(_address, (uint8_t)9);
  
  if (Wire.available() >= 9) {
    uint8_t distL = Wire.read();
    uint8_t distH = Wire.read();
    uint8_t strengthL = Wire.read();
    uint8_t strengthH = Wire.read();
    uint8_t tempL = Wire.read();
    uint8_t tempH = Wire.read();
    
    // Vyčtení zbytku
    Wire.read(); Wire.read(); Wire.read();

    _dist = distL + (distH * 256);
    _strength = strengthL + (strengthH * 256);
    _temp = (tempL + (tempH * 256)) / 100.0;
    
    return true;
  }
  return false;
}

uint16_t TFLuna::getDistance() { return _dist; }
uint16_t TFLuna::getSignalStrength() { return _strength; }
float TFLuna::getTemperature() { return _temp; }

// --- Implementace nastavení ---

bool TFLuna::setFrameRate(uint16_t fps) {
  if (fps < 1) fps = 1;
  if (fps > 250) fps = 250;
  // Registr 0x03 = Frame Rate
  return writeReg(0x03, fps);
}

bool TFLuna::setI2CAddress(uint8_t newAddr) {
  // Registr 0x22 = Slave ID
  if (writeRegByte(0x22, newAddr)) {
    // Pokud se zápis povedl, musíme si novou adresu pamatovat v knihovně
    _address = newAddr;
    return true;
  }
  return false;
}

bool TFLuna::saveSettings() {
  // Registr 0x20 = Save Settings (write 1)
  return writeRegByte(0x20, 0x01);
}

bool TFLuna::factoryReset() {
  // Registr 0x3C = Restore Factory Settings (write 1)
  if (writeRegByte(0x3C, 0x01)) {
    // Po resetu se adresa vrátí na 0x10
    _address = 0x10;
    return true;
  }
  return false;
}

// --- Privátní funkce ---

bool TFLuna::writeReg(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value & 0xFF);
  Wire.write((value >> 8) & 0xFF);
  return (Wire.endTransmission() == 0);
}

bool TFLuna::writeRegByte(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_address);
  Wire.write(reg);
  Wire.write(value);
  return (Wire.endTransmission() == 0);
}