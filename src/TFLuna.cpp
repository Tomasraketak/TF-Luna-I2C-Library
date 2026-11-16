#include "TFLuna.h"

TFLuna::TFLuna(uint8_t address) {
  _wire = &Wire;
  _address = address;
  _pendingAddress = 0;
  _dist = 0;
  _strength = 0;
  _temp = 0.0;
  _tick = 0;
}

bool TFLuna::begin(int sdaPin, int sclPin, TwoWire &wire) {
  _wire = &wire;
#if defined(ARDUINO_ARCH_RP2040)
  // arduino-pico: pin musí patřit ke zvolené sběrnici, jinak setSDA/setSCL selže
  if (!_wire->setSDA(sdaPin)) return false;
  if (!_wire->setSCL(sclPin)) return false;
  _wire->begin();
#elif defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  _wire->begin(sdaPin, sclPin);
#else
  // Desky s pevnými I2C piny (AVR, SAMD, ...) - zadané piny se ignorují
  (void)sdaPin;
  (void)sclPin;
  _wire->begin();
#endif
  return isConnected();
}

bool TFLuna::begin(TwoWire &wire) {
  _wire = &wire;
  _wire->begin();
  return isConnected();
}

bool TFLuna::read() {
  uint8_t buf[8];
  if (!readRegs(TFLUNA_REG_DIST_L, buf, sizeof(buf))) return false;

  _dist = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
  _strength = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
  // Teplota je se znaménkem v setinách °C
  _temp = (int16_t)((uint16_t)buf[4] | ((uint16_t)buf[5] << 8)) / 100.0;
  _tick = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
  return true;
}

uint16_t TFLuna::getDistance() { return _dist; }
uint16_t TFLuna::getSignalStrength() { return _strength; }
float TFLuna::getTemperature() { return _temp; }
uint16_t TFLuna::getTimestamp() { return _tick; }

bool TFLuna::isValid() {
  return _strength >= TFLUNA_MIN_SIGNAL && _strength != TFLUNA_SIGNAL_SATURATED;
}

// --- Diagnostika ---

bool TFLuna::isConnected() {
  // Ověří podpis "LUNA" - odliší TF-Lunu od jiného zařízení na stejné adrese
  uint8_t sig[4];
  if (!readRegs(TFLUNA_REG_SIGNATURE, sig, sizeof(sig))) return false;
  return sig[0] == 'L' && sig[1] == 'U' && sig[2] == 'N' && sig[3] == 'A';
}

uint8_t TFLuna::getAddress() { return _address; }

bool TFLuna::getFirmwareVersion(uint8_t &major, uint8_t &minor, uint8_t &revision) {
  uint8_t buf[3];
  if (!readRegs(TFLUNA_REG_VERSION_REV, buf, sizeof(buf))) return false;
  revision = buf[0];
  minor = buf[1];
  major = buf[2];
  return true;
}

bool TFLuna::getSerialNumber(char *buffer, size_t bufferSize) {
  const uint8_t snLength = 14;
  if (buffer == nullptr || bufferSize < (size_t)snLength + 1) return false;
  if (!readRegs(TFLUNA_REG_SN, (uint8_t *)buffer, snLength)) return false;
  buffer[snLength] = '\0';
  return true;
}

uint16_t TFLuna::getFrameRate() {
  uint8_t buf[2];
  if (!readRegs(TFLUNA_REG_FPS_L, buf, sizeof(buf))) return 0;
  return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

// --- Nastavení ---

bool TFLuna::setFrameRate(uint16_t fps) {
  if (fps < 1) fps = 1;
  if (fps > 250) fps = 250;
  return writeReg(TFLUNA_REG_FPS_L, fps);  // zapíše 0x26 (L) a 0x27 (H)
}

bool TFLuna::setI2CAddress(uint8_t newAddr) {
  if (newAddr < TFLUNA_MIN_ADDRESS || newAddr > TFLUNA_MAX_ADDRESS) return false;
  if (newAddr == _address) return true;
  // Na nové adrese už něco je - po změně by na sběrnici byla kolize
  if (probe(newAddr)) return false;

  if (!writeRegByte(TFLUNA_REG_SLAVE_ADDR, newAddr)) return false;
  delay(10);

  if (probe(newAddr)) {
    // Senzor přešel na novou adresu okamžitě
    _address = newAddr;
    _pendingAddress = 0;
  } else {
    // Senzor zůstává na staré adrese, nová platí po saveSettings() + reboot()
    _pendingAddress = newAddr;
  }
  return true;
}

bool TFLuna::saveSettings() {
  bool ok = writeRegByte(TFLUNA_REG_SAVE, 0x01);
  if (!ok && _pendingAddress != 0 &&
      writeRegByteTo(_pendingAddress, TFLUNA_REG_SAVE, 0x01)) {
    // Senzor už mezitím odpovídá na nové adrese
    _address = _pendingAddress;
    _pendingAddress = 0;
    ok = true;
  }
  if (ok) delay(50);  // čas na zápis do flash
  return ok;
}

bool TFLuna::reboot(uint16_t timeoutMs) {
  // Senzor se může restartovat dřív, než potvrdí zápis - výsledek zápisu nevadí
  writeRegByte(TFLUNA_REG_REBOOT, 0x02);
  delay(100);
  return waitForSensor(timeoutMs);
}

bool TFLuna::factoryReset() {
  if (!writeRegByte(TFLUNA_REG_FACTORY_RESET, 0x01)) return false;
  delay(50);
  if (_address != TFLUNA_DEFAULT_ADDRESS) {
    if (probe(TFLUNA_DEFAULT_ADDRESS) && !probe(_address)) {
      _address = TFLUNA_DEFAULT_ADDRESS;
      _pendingAddress = 0;
    } else {
      _pendingAddress = TFLUNA_DEFAULT_ADDRESS;  // projeví se po reboot()
    }
  }
  return true;
}

bool TFLuna::enable() { return writeRegByte(TFLUNA_REG_ENABLE, 0x01); }
bool TFLuna::disable() { return writeRegByte(TFLUNA_REG_ENABLE, 0x00); }

bool TFLuna::setTriggerMode(bool enabled) {
  return writeRegByte(TFLUNA_REG_MODE, enabled ? 0x01 : 0x00);
}

bool TFLuna::trigger() {
  if (!writeRegByte(TFLUNA_REG_TRIGGER, 0x01)) return false;
  delay(5);  // čas na dokončení měření
  return true;
}

bool TFLuna::setLowPower(bool enabled) {
  return writeRegByte(TFLUNA_REG_LOW_POWER, enabled ? 0x01 : 0x00);
}

// --- Privátní funkce ---

bool TFLuna::probe(uint8_t address) {
  _wire->beginTransmission(address);
  return _wire->endTransmission() == 0;
}

bool TFLuna::waitForSensor(uint16_t timeoutMs) {
  unsigned long start = millis();
  do {
    if (_pendingAddress != 0) {
      uint8_t oldAddr = _address;
      _address = _pendingAddress;
      if (isConnected()) {
        _pendingAddress = 0;
        return true;
      }
      _address = oldAddr;
    }
    if (isConnected()) {
      // Senzor zůstal na staré adrese (změna nebyla uložena)
      _pendingAddress = 0;
      return true;
    }
    delay(20);
  } while (millis() - start < timeoutMs);
  return false;
}

bool TFLuna::readRegs(uint8_t reg, uint8_t *buffer, uint8_t length) {
  _wire->beginTransmission(_address);
  _wire->write(reg);
  if (_wire->endTransmission() != 0) return false;

  uint8_t received = _wire->requestFrom(_address, length);
  if (received < length) {
    while (_wire->available()) _wire->read();  // vyprázdnit neúplná data
    return false;
  }
  for (uint8_t i = 0; i < length; i++) buffer[i] = _wire->read();
  return true;
}

bool TFLuna::writeReg(uint8_t reg, uint16_t value) {
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value & 0xFF);
  _wire->write((value >> 8) & 0xFF);
  return (_wire->endTransmission() == 0);
}

bool TFLuna::writeRegByte(uint8_t reg, uint8_t value) {
  return writeRegByteTo(_address, reg, value);
}

bool TFLuna::writeRegByteTo(uint8_t address, uint8_t reg, uint8_t value) {
  _wire->beginTransmission(address);
  _wire->write(reg);
  _wire->write(value);
  return (_wire->endTransmission() == 0);
}
