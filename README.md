# TF Luna I2C Library

Jednoduchá a efektivní Arduino knihovna pro LiDAR senzor **TF Luna** připojený přes sběrnici **I2C**.

Knihovna byla vyvinuta a testována primárně na **Raspberry Pi Pico 2** (jádro [arduino-pico](https://github.com/earlephilhower/arduino-pico)), ale funguje s většinou Arduino desek využívajících knihovnu `Wire` (ESP32, ESP8266, AVR, SAMD…).

## ✨ Vlastnosti
* **Čtení:** vzdálenost (cm), síla signálu, teplota čipu (°C), časová značka.
* **Kontrola kvality:** `isValid()` pozná nespolehlivé měření (slabý nebo přesycený signál).
* **Detekce senzoru:** `begin()` a `isConnected()` ověří, že na adrese je opravdu TF-Luna.
* **Rychlé čtení:** jedno čtení trvá jen jednu krátkou I2C transakci (žádné čekání v knihovně).
* **Konfigurace:**
    * Změna frame rate (1 Hz – 250 Hz).
    * Změna I2C adresy (umožňuje připojit více senzorů).
    * Trigger režim (měření na povel), zapnutí/vypnutí měření, úsporný režim.
    * Ukládání nastavení do paměti senzoru, restart, tovární reset.
* **Diagnostika:** verze firmwaru, sériové číslo, aktuální frame rate.

## 🔌 Zapojení (Raspberry Pi Pico / Pico 2)

| TF Luna pin | Funkce | Raspberry Pi Pico pin |
| :--- | :--- | :--- |
| **1** | +5V napájení | VBUS (5V) * |
| **2** | SDA (I2C data) | GP4 (I2C0 SDA) |
| **3** | SCL (I2C hodiny) | GP5 (I2C0 SCL) |
| **4** | GND | GND |
| **5** | Výběr režimu | **GND** (= I2C režim) |
| **6** | Multifunkční výstup | nezapojeno |

*\* TF Luna vyžaduje 5 V napájení, logika I2C je 3,3 V, takže je přímo kompatibilní s Pico.*

> **Režim I2C:** TF Luna komunikuje přes I2C **jen když je pin 5 připojen na GND**. Když je pin 5 volný, senzor běží v režimu UART a přes I2C neodpovídá. Tovární reset tento režim nemění.

> **Pull-up rezistory:** Na SDA a SCL doporučujeme externí pull-upy cca **4,7 kΩ na 3,3 V**. Interní pull-upy mikrokontroléru jsou slabé a při delších vodičích nemusí stačit.

> **Pico a sběrnice:** Na RP2040/RP2350 patří každý pin k určité sběrnici. GP4/GP5 jsou `Wire` (I2C0). Pro piny I2C1 (např. GP2/GP3) použij `luna.begin(2, 3, Wire1)`. Když piny ke sběrnici nepatří, `begin()` vrátí `false`.

## 🚀 Příklad použití

```cpp
#include <TFLuna.h>

#define SDA_PIN 4
#define SCL_PIN 5

TFLuna luna; // výchozí adresa 0x10

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);

  if (!luna.begin(SDA_PIN, SCL_PIN)) {
    Serial.println("Senzor nenalezen!");
  }
}

void loop() {
  if (luna.read() && luna.isValid()) {
    Serial.print("Vzdalenost: ");
    Serial.print(luna.getDistance());
    Serial.print(" cm | Teplota: ");
    Serial.println(luna.getTemperature());
  }
  delay(20);
}
```

Další příklady najdeš ve složce `examples`:
* **BasicReading:** čtení hodnot s kontrolou platnosti.
* **Configuration:** verze firmwaru, sériové číslo, nastavení frame rate.
* **TF_Luna_test:** změna I2C adresy, uložení a restart.

## 📖 API

| Funkce | Popis |
| :--- | :--- |
| `TFLuna(addr = 0x10)` | Vytvoří instanci pro senzor na dané adrese. |
| `bool begin(sda, scl, wire = Wire)` | Nastaví piny, spustí I2C a ověří senzor. |
| `bool begin(wire = Wire)` | Spustí I2C na výchozích pinech a ověří senzor. |
| `bool read()` | Přečte nové hodnoty. `false` = chyba komunikace. |
| `getDistance()` | Vzdálenost v cm. |
| `getSignalStrength()` | Síla signálu. |
| `getTemperature()` | Teplota čipu v °C. |
| `getTimestamp()` | Interní čas senzoru v ms. |
| `bool isValid()` | `true`, když je síla signálu ≥ 100 a není přesycená (65535). |
| `bool isConnected()` | Odpovídá senzor na aktuální adrese? |
| `getAddress()` | Adresa, kterou knihovna právě používá. |
| `getFirmwareVersion(major, minor, rev)` | Verze firmwaru senzoru. |
| `getSerialNumber(buf, size)` | Sériové číslo (buffer aspoň 15 znaků). |
| `getFrameRate()` / `setFrameRate(fps)` | Čtení / nastavení frekvence (1–250 Hz). |
| `setI2CAddress(addr)` | Změna adresy (0x08–0x77). Kontroluje, jestli adresa není obsazená. |
| `saveSettings()` | Uloží nastavení do trvalé paměti senzoru. |
| `reboot(timeoutMs = 1000)` | Restartuje senzor a počká, až odpoví. Po změně adresy se přepne na novou. |
| `factoryReset()` | Tovární nastavení (0x10, 100 Hz). Projeví se po `reboot()`. |
| `enable()` / `disable()` | Zapnutí / vypnutí měření. |
| `setTriggerMode(bool)` / `trigger()` | Měření na povel: `trigger()` a pak `read()`. |
| `setLowPower(bool)` | Úsporný režim. |

### Trvalá změna adresy

```cpp
luna.setI2CAddress(0x11);  // změna adresy
luna.saveSettings();        // uložení do paměti senzoru
luna.reboot();              // restart, knihovna dál komunikuje na 0x11
```

Nastavení (adresa, frame rate, režimy) bez `saveSettings()` platí jen do vypnutí senzoru.

## 📝 Změny

### 1.1.0
* **Oprava:** `setFrameRate()` zapisoval do registru jen pro čtení. Teď správně zapisuje 0x26/0x27.
* **Oprava:** `factoryReset()` zapisoval do registru podpisu. Teď správně zapisuje 0x29.
* **Oprava:** pinout v README (SDA = pin 2, SCL = pin 3, GND = pin 4) a popis přepnutí do I2C (pin 5 na GND).
* **Oprava:** záporné teploty se teď čtou správně.
* `begin()` ověřuje, že senzor odpovídá. Funguje i mimo RP2040 (ESP32, AVR…), přidán výběr sběrnice (`Wire1`).
* `setI2CAddress()` kontroluje rozsah a obsazenost adresy. `saveSettings()` zvládne adresu změněnou za běhu.
* Nové funkce: `isValid()`, `isConnected()`, `reboot()`, `getAddress()`, `getTimestamp()`, `getFrameRate()`, `getFirmwareVersion()`, `getSerialNumber()`, `enable()`, `disable()`, `setTriggerMode()`, `trigger()`, `setLowPower()`.
* Nové příklady a `keywords.txt`.
* Zpětně kompatibilní s verzí 1.0.0, stávající sketche fungují beze změn.
