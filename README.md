# NanoSubGHz

> **Sub-GHz RF Tool for Arduino Nano** — Sniff, Replay, and Analyze 315/433 MHz ASK Signals

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20Nano-blue.svg)](https://www.arduino.cc)
[![Status](https://img.shields.io/badge/Status-Educational-orange.svg)]()

**Author:** Sassiz.Root

---

## ⚠️ DISCLAIMER — READ FIRST

> **This project is provided for EDUCATIONAL and RESEARCH purposes ONLY.**
>
> Jamming, intercepting, or replaying RF signals belonging to others may be
> **ILLEGAL** in your country and can disrupt critical systems (car alarms,
> garage doors, industrial controls, medical devices).
>
> **Use only on devices you own**, in a controlled environment, and in
> compliance with your local laws. The author assumes **no responsibility**
> for misuse. See [DISCLAIMER.md](DISCLAIMER.md) for full terms.

---

## ✨ Features

| Feature | Description |
|---|---|
| 📡 **Sniff 433 MHz** | Capture ASK/OOK codes on 433.92 MHz |
| 📡 **Sniff 315 MHz** | Capture ASK/OOK codes on 315 MHz |
| 💾 **SD Logging** | Auto-save every captured code to CSV with RSSI & protocol |
| 📖 **View Codes** | Browse captured codes with pagination |
| 🔁 **Replay** | Re-transmit any saved code |
| 🗑️ **Delete** | Remove individual codes or wipe all data |
| 🌀 **Jam** | Disrupt 315 + 433 bands (⚠️ legal warning) |
| 🔢 **Brute-Force** | Iterate 12-bit or 16-bit codes |
| 🎚️ **RSSI** | 0–5 estimated signal strength via AGC pin |
| 🔌 **Safe Shutdown** | Flush SD, power down modules, prompt for safe unplug |

---

## 🧰 Hardware Requirements

| Component | Qty | Notes |
|---|---|---|
| Arduino Nano (ATmega328P) | 1 | 16 MHz clone is fine |
| LCD 1602 + I2C backpack | 1 | I2C address `0x27` (change if `0x3F`) |
| SD Card Module | 1 | SPI, FAT16/FAT32, ≤32 GB |
| Micro SD Card | 1 | Formatted FAT32 |
| RX ASK 433 MHz | 1 | e.g. **RXB6** for AGC pin |
| RX ASK 315 MHz | 1 | e.g. **RXB6** for AGC pin |
| TX ASK 433 MHz | 1 | e.g. FS1000A |
| TX ASK 315 MHz | 1 | e.g. FS1000A |
| Push Buttons | 5 | LEFT, UP, DOWN, SELECT, BACK |
| Antennas | 4 | 17.3 cm wire for 433, 23.8 cm for 315 |
| Breadboard + Jumper wires | — | — |

> 💡 **Recommended:** Use **RXB6** receivers instead of the cheaper **XY-MK-5V**.
> RXB6 modules provide an **AGC pin** that enables the RSSI feature, and offer
> far better sensitivity and selectivity.

---

## 🔌 Wiring Diagram

### Pin Map

| Module | Module Pin | Nano Pin | Notes |
|---|---|---|---|
| **LCD 1602 (I2C)** | SDA | **A4** | I2C data |
| | SCL | **A5** | I2C clock |
| | VCC / GND | **5V / GND** | |
| **SD Card** | CS | **D10** | SPI CS |
| | MOSI | **D11** | SPI |
| | MISO | **D12** | SPI |
| | SCK | **D13** | SPI |
| | VCC / GND | **5V / GND** | |
| **RX 433** | DATA | **D2** | INT0 |
| | VCC / GND | **5V / GND** | |
| | AGC | **A6** | Analog only (RXB6 only) |
| **RX 315** | DATA | **D3** | INT1 |
| | VCC / GND | **5V / GND** | |
| | AGC | **A7** | Analog only (RXB6 only) |
| **TX 433** | DATA | **D4** | Transmit 433 |
| | VCC / GND | **5V / GND** | |
| **TX 315** | DATA | **D5** | Transmit 315 |
| | VCC / GND | **5V / GND** | |
| **Button LEFT** | Leg1 / Leg2 | **D6 / GND** | `INPUT_PULLUP` |
| **Button UP** | Leg1 / Leg2 | **D8 / GND** | |
| **Button DOWN** | Leg1 / Leg2 | **D9 / GND** | |
| **Button SELECT** | Leg1 / Leg2 | **A0 / GND** | |
| **Button BACK** | Leg1 / Leg2 | **A1 / GND** | |

**Free pins (for future extension):** `D7`, `A2`, `A3`

> ⚠️ **Antenna lengths:**
> - **433 MHz** → λ/4 ≈ **17.3 cm** wire
> - **315 MHz** → λ/4 ≈ **23.8 cm** wire
>
> Solder a straight wire to the ANT pad of each module. Without an antenna,
> range drops to a few centimeters.

---

## 💻 Software Installation

### 1. Install Arduino IDE
Download from [arduino.cc/en/software](https://www.arduino.cc/en/software).

### 2. Install required libraries
Open **Sketch → Include Library → Manage Libraries** and install:

| Library | Author | Purpose |
|---|---|---|
| **RCSwitch** | sui77 | OOK/ASK encode/decode |
| **LiquidCrystal I2C** | Frank de Brabander | LCD I2C driver |
| **SD** | Arduino / SparkFun | Built-in (verify installed) |
| **SPI** | Arduino | Built-in |

### 3. Upload firmware
1. Open `firmware/NanoSubGHz.ino`
2. Select **Tools → Board → Arduino Nano**
3. Select **Tools → Processor → ATmega328P** (or Old Bootloader if needed)
4. Select the correct COM port
5. Click **Upload**

### 4. Prepare the SD card
- Format as **FAT32**
- No files needed — the firmware creates them on first boot

---

## 🎮 Usage

See [docs/usage.md](docs/usage.md) for a complete guide.

### Quick start

1. Power on → splash `NanoSubGhz` / `by Sassiz.Root` → `Booting...` → menu
2. Navigate with **UP / DOWN**, confirm with **SELECT**, go back with **LEFT**
3. Choose **Sniff** → pick frequency (433 / 315) → SELECT → start capturing
4. Every captured code auto-saves to `/codes_433.csv` or `/codes_315.csv`
5. Press **LEFT** to exit sniff

### Menu structure

```
┌─────────────────────────────┐
│ >1.Sniff                    │
│  2.View Code                │
│  3.Jam                      │
│  4.Brute-Force              │
│  5.Clear Data               │
│  6.Shutdown                 │
└─────────────────────────────┘
```

---

## 📂 SD Card File Format

Each captured code is stored as a single CSV line:

```
<code_hex>,<protocol>,<bits>,<rssi>,<modulation>
```

Example:

```
A1B2C3,1,24,4,ASK
F0E1D2,2,12,3,ASK
```

| Field | Meaning |
|---|---|
| `code_hex` | Received code (hexadecimal) |
| `protocol` | RCSwitch protocol number (1–12) |
| `bits` | Bit length (typically 12, 24, or 32) |
| `rssi` | Estimated signal strength (0–5) |
| `modulation` | Fixed `ASK` for these modules |

To export, remove the SD card and open with any spreadsheet app.

---

## 📊 Performance Notes

| Metric | Value |
|---|---|
| Sniff rate (per band) | ~ 10–40 codes/sec (protocol dependent) |
| Brute-force speed (12-bit) | ~ 400 codes/sec → full sweep in ~10 s |
| Brute-force speed (16-bit) | ~ 400 codes/sec → full sweep in ~30 min |
| Codes per SD file | ~ 40,000 (limited by FAT, not by code) |
| Max range (with antenna) | ~ 10–30 m line-of-sight (433) |
| LCD refresh | on-demand (no flicker) |

---

## 🐛 Known Limitations

- **ASK modules (XY-MK-5V, FS1000A)** are extremely noisy. Capture quality
  is far below SDR or Flipper Zero.
- **No true simultaneous sniffing** — the firmware uses one receiver at a
  time to save RAM and avoid interference. If you need both bands at once,
  upgrade to **CC1101**.
- **RSSI requires RXB6**. On cheap XY-MK-5V receivers, RSSI will read `0`.
- **Only OOK/ASK** — FSK, GFSK, and Manchester encoded signals are **not**
  supported by these modules.

---

## 🚀 Roadmap

- [ ] Support for `.sub` file format (Flipper Zero compatible)
- [ ] CC1101 module support (true simultaneous, FSK)
- [ ] Buzzer / LED indicator
- [ ] Timestamp via RTC (DS3231)
- [ ] Bilingual UI (English / فارسی)

---

## 🤝 Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you'd like to change.

Before submitting a PR:
- Read [DISCLAIMER.md](DISCLAIMER.md)
- Keep SRAM usage under 1.5 KB
- Test on real hardware
- Add screenshots where useful

---

## 📜 License

[MIT](LICENSE) © Sassiz.Root

---

## 🙏 Credits

- **RCSwitch** by [sui77](https://github.com/sui77/rc-switch)
- **LiquidCrystal_I2C** by Frank de Brabander
- Inspired by [Flipper Zero](https://flipperzero.one) Sub-GHz module
- Community projects: **Universal Radio Hacker**, **rfcat**, **rpitx**

---

## 📸 Screenshots

> *(Add your own photos of the breadboard build and LCD menus here)*

| Menu | Sniff | Detail |
|---|---|---|
| _coming soon_ | _coming soon_ | _coming soon_ |

---

**⭐ If this project helped you learn, please star the repo!**
