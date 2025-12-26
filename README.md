# TF Luna I2C Library

Jednoduchá a efektivní Arduino knihovna pro LiDAR senzor **TF Luna** připojený přes sběrnici **I2C**. 

Knihovna byla vyvinuta a testována primárně na **Raspberry Pi Pico 2**, ale je kompatibilní s většinou Arduino desek využívajících knihovnu `Wire`.

## ✨ Vlastnosti
* **Základní čtení:** Vzdálenost (cm), Síla signálu, Teplota čipu (°C).
* **Non-blocking:** Čtení dat neblokuje běh programu.
* **Konfigurace:** * Změna Frame Rate (1 Hz – 250 Hz).
    * Změna I2C adresy (umožňuje připojení více senzorů).
    * Ukládání nastavení do paměti senzoru.
    * Tovární reset.

## 🔌 Zapojení (Raspberry Pi Pico 2)

Doporučené zapojení pro I2C komunikaci:

| TF Luna Pin | Funkce | Raspberry Pi Pico 2 Pin |
| :--- | :--- | :--- |
| **1 (VCC)** | 5V Napájení | VBUS (5V) * |
| **2 (GND)** | Zem | GND |
| **3 (SDA)** | I2C Data | GP4 (SDA0) |
| **4 (SCL)** | I2C Hodiny | GP5 (SCL0) |

*\*TF Luna vyžaduje 5V napájení, ale logika I2C je 3.3V kompatibilní.*

> **Upozornění:** TF Luna je z výroby v UART režimu. Před použitím se ujistěte, že je přepnuta na I2C.

## 🚀 Příklad použití

```cpp
#include <Wire.h>
#include "TFLuna.h"

// Definice pinů pro Raspberry Pi Pico
#define SDA_PIN 4
#define SCL_PIN 5

// Vytvoření instance (defaultní adresa 0x10)
TFLuna luna;

void setup() {
  Serial.begin(115200);
  while(!Serial) delay(10);

  // Inicializace
  luna.begin(SDA_PIN, SCL_PIN);
  
  Serial.println("Senzor pripraven");
}

void loop() {
  if (luna.read()) {
    Serial.print("Vzdalenost: ");
    Serial.print(luna.getDistance());
    Serial.print(" cm | Teplota: ");
    Serial.println(luna.getTemperature());
  }
  
  delay(20);
}