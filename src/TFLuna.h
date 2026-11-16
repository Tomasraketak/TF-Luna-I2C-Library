#ifndef TFLuna_h
#define TFLuna_h

#include <Arduino.h>
#include <Wire.h>

// --- Registry TF-Luna (I2C) podle datasheetu Benewake ---
#define TFLUNA_REG_DIST_L        0x00  // vzdálenost [cm], 0x00-0x01
#define TFLUNA_REG_AMP_L         0x02  // síla signálu, 0x02-0x03
#define TFLUNA_REG_TEMP_L        0x04  // teplota [0.01 °C], 0x04-0x05
#define TFLUNA_REG_TICK_L        0x06  // časová značka [ms], 0x06-0x07
#define TFLUNA_REG_VERSION_REV   0x0A  // verze firmwaru: 0x0A rev, 0x0B minor, 0x0C major
#define TFLUNA_REG_SN            0x10  // sériové číslo, 14 ASCII znaků (0x10-0x1D)
#define TFLUNA_REG_SAVE          0x20  // zapsat 0x01 = uložit nastavení
#define TFLUNA_REG_REBOOT        0x21  // zapsat 0x02 = restart
#define TFLUNA_REG_SLAVE_ADDR    0x22  // I2C adresa (0x08-0x77)
#define TFLUNA_REG_MODE          0x23  // 0 = kontinuální, 1 = trigger
#define TFLUNA_REG_TRIGGER       0x24  // zapsat 0x01 = jedno měření (v trigger režimu)
#define TFLUNA_REG_ENABLE        0x25  // 0 = vypnout měření, 1 = zapnout
#define TFLUNA_REG_FPS_L         0x26  // frame rate [Hz], 0x26-0x27
#define TFLUNA_REG_LOW_POWER     0x28  // 0 = normální, 1 = úsporný režim
#define TFLUNA_REG_FACTORY_RESET 0x29  // zapsat 0x01 = tovární nastavení
#define TFLUNA_REG_SIGNATURE     0x3C  // "LUNA", 0x3C-0x3F

#define TFLUNA_DEFAULT_ADDRESS   0x10
#define TFLUNA_MIN_ADDRESS       0x08
#define TFLUNA_MAX_ADDRESS       0x77

// Pod touto silou signálu (nebo při 0xFFFF = přesycení) je vzdálenost nespolehlivá
#define TFLUNA_MIN_SIGNAL        100
#define TFLUNA_SIGNAL_SATURATED  0xFFFF

class TFLuna {
  public:
    // Konstruktor - defaultně adresa 0x10, nic se na senzoru nemění (ODR zůstává 100 Hz)
    TFLuna(uint8_t address = TFLUNA_DEFAULT_ADDRESS);

    // Inicializace sběrnice na zadaných pinech a ověření, že senzor odpovídá.
    // Vrací true, pokud byl senzor nalezen. Na RP2040/RP2350 musí piny patřit
    // ke zvolené sběrnici (Wire = I2C0, např. GP4/GP5; Wire1 = I2C1, např. GP2/GP3).
    bool begin(int sdaPin, int sclPin, TwoWire &wire = Wire);

    // Inicializace na výchozích pinech dané sběrnice (nebo na sběrnici,
    // kterou už sis nastavil sám).
    bool begin(TwoWire &wire = Wire);

    // Přečte vzdálenost, sílu signálu, teplotu a časovou značku.
    // Vrací true, pokud komunikace proběhla (neříká nic o kvalitě měření - viz isValid()).
    bool read();

    // Čtení naměřených hodnot (z posledního úspěšného read())
    uint16_t getDistance();       // cm
    uint16_t getSignalStrength(); // libovolná jednotka
    float getTemperature();       // °C
    uint16_t getTimestamp();      // interní čas senzoru [ms], přetéká po 65535

    // true, pokud je poslední měření spolehlivé (síla signálu >= 100 a není přesycená)
    bool isValid();

    // --- Diagnostika ---
    bool isConnected();           // odpovídá senzor na aktuální adrese?
    uint8_t getAddress();         // adresa, kterou knihovna právě používá
    bool getFirmwareVersion(uint8_t &major, uint8_t &minor, uint8_t &revision);
    bool getSerialNumber(char *buffer, size_t bufferSize); // buffer alespoň 15 bajtů
    uint16_t getFrameRate();      // aktuální frame rate ze senzoru, 0 = chyba

    // --- Nastavení senzoru ---

    // Změna frekvence (1-250 Hz). Pokud nezavoláš, je tam default 100 Hz.
    bool setFrameRate(uint16_t fps);

    // Změna I2C adresy (0x08-0x77). Vrací false při neplatné adrese, když je adresa
    // obsazená jiným zařízením, nebo když senzor neodpovídá. Pro trvalou změnu
    // zavolej saveSettings() a pak reboot().
    bool setI2CAddress(uint8_t newAddr);

    // Uloží aktuální nastavení (frekvenci i adresu) do trvalé paměti
    bool saveSettings();

    // Restartuje senzor a počká, až znovu odpoví (max. timeoutMs).
    // Po změně adresy se automaticky přepne na novou adresu.
    bool reboot(uint16_t timeoutMs = 1000);

    // Obnoví tovární nastavení (adresa 0x10, 100 Hz). Režim rozhraní (I2C/UART)
    // se tím nemění - ten určuje pin 5 senzoru. Projeví se po reboot().
    bool factoryReset();

    // Zapnutí / vypnutí měření (vypnutý senzor méně hřeje a méně spotřebuje)
    bool enable();
    bool disable();

    // Trigger režim: senzor měří jen na povel trigger(), pak zavolej read()
    bool setTriggerMode(bool enabled);
    bool trigger();

    // Úsporný režim (senzor sníží frekvenci měření)
    bool setLowPower(bool enabled);

  private:
    TwoWire *_wire;
    uint8_t _address;
    uint8_t _pendingAddress;      // adresa, na kterou senzor přejde po restartu (0 = žádná)
    uint16_t _dist;
    uint16_t _strength;
    float _temp;
    uint16_t _tick;

    bool probe(uint8_t address);
    bool waitForSensor(uint16_t timeoutMs);
    bool readRegs(uint8_t reg, uint8_t *buffer, uint8_t length);

    // Pomocné funkce pro zápis
    bool writeReg(uint8_t reg, uint16_t value);
    bool writeRegByte(uint8_t reg, uint8_t value);
    bool writeRegByteTo(uint8_t address, uint8_t reg, uint8_t value);
};

#endif
