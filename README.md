# iTambaFirmware

Firmware for the **iTamba** embedded device, developed in C/C++ for microcontrollers. This project handles sensor interfaces, communication protocols, power management, and optional firmware updates over the air (OTA).

---

## ⚙️ Features

- 🧠 Driver support for various sensors (e.g. temperature, humidity, accelerometer)
- 🔗 I²C, SPI, UART, and GPIO-based communication
- 🌐 Connectivity layer (e.g. Wi‑Fi / BLE / LoRaWAN)
- ⚡ Low-power / sleep modes and battery-friendly operations
- 📦 Optional OTA firmware update support
- 🔧 Modular folder structure: `drivers/`, `middlewares/`, `app/`, `config/`

---

## 🚀 Getting Started

### Requirements

- ARM Cortex-M / AVR (or your chosen MCU)
- GCC-based toolchain (arm-none-eabi-gcc or avr-gcc)
- Make or CMake build system
- Firmware flashing tool (STM32CubeProgrammer, avrdude, esptool, etc.)

### Installation & Build

Clone and navigate:

```bash
git clone https://github.com/DevMiguelPinheiro/iTambaFirmware.git
cd iTambaFirmware
