# NanoSubGHz — Usage Guide

Complete operating manual for the NanoSubGHz device.

---

## Table of Contents

1. [First Power-On](#first-power-on)
2. [Button Navigation](#button-navigation)
3. [Menu Reference](#menu-reference)
4. [Sniff Mode](#1-sniff)
5. [View Codes](#2-view-code)
6. [Jam Mode](#3-jam)
7. [Brute-Force](#4-brute-force)
8. [Clear Data](#5-clear-data)
9. [Shutdown](#6-shutdown)
10. [SD Card Files](#sd-card-files)
11. [Tips & Best Practices](#tips--best-practices)
12. [Troubleshooting](#troubleshooting)

---

## First Power-On

When you first power the device (via USB or 5V pin), the following sequence
runs:

1. **Splash screen** (~2 s):
   ```
   NanoSubGhz
   by Sassiz.Root
   ```

2. **Boot message** (~1.2 s):
   ```
   Booting...
   ```

3. **Main menu** appears. If the SD card fails to initialize, you will see
   `SD Init Fail` — power off, check wiring, reseat the SD, and retry.

> 💡 The device does **not** need a PC. Once flashed, it runs standalone.

---

## Button Navigation

| Button | Nano Pin | Typical Role |
|---|---|---|
| **UP** | D8 | Move up / increase |
| **DOWN** | D9 | Move down / decrease |
| **SELECT** | A0 | Confirm / enter / action |
| **BACK** | A1 | Cancel / delete / back |
| **LEFT** | D6 | Exit to main menu |

### General rules
- **LEFT** always returns to the **previous screen**.
- **SELECT** accepts the highlighted option.
- **BACK** cancels without saving (except in the detail view, where it
  **deletes** the selected code — see warning below).

> ⚠️ **Warning:** In the *Code Detail* view, **BACK deletes the code**.
> If you only want to leave, press **LEFT** instead.

---

## Menu Reference

```
┌─────────────────────────────┐
│ >1.Sniff                    │   ← 6 items, UP/DOWN to scroll
│  2.View Code                │
│  3.Jam                      │
│  4.Brute-Force              │
│  5.Clear Data               │
│  6.Shutdown                 │
└─────────────────────────────┘
```

---

## 1. Sniff

**Purpose:** Capture incoming ASK/OOK codes on the selected frequency and
store them on the SD card.

### Steps

1. Menu → **1.Sniff** → **SELECT**
2. Choose frequency:
   ```
   Sniff Frequency
   >433   315
   ```
   Use **UP / DOWN** to toggle, **SELECT** to confirm, **LEFT** to cancel.
3. Device enters sniffing mode:
   ```
   Sniff 433 MHz
   L=Exit
   ```
4. When a code is received, a **brief overlay** appears (~0.6 s):
   ```
   433MHz RSSI:4
   C:A1B2C3
   ```
   The code is **automatically saved** before the screen returns to
   sniff mode.
5. Press **LEFT** to exit.

### What is captured

- Received code (hex)
- Protocol number (1–12, per RCSwitch)
- Bit length (typically 12 / 24 / 32)
- Estimated RSSI (0–5, requires RXB6 AGC pin)
- Modulation: `ASK` (fixed)

### Notes
- Each capture consumes one SD write (~5–20 ms) — during that window, no
  new codes are captured.
- **Range depends on antenna.** Without antenna: a few cm. With 17.3 cm
  wire: 10–30 m.
- There is **no minimum signal check** — noise may appear in the CSV.
  Delete obvious noise lines from *View Code*.

---

## 2. View Code

**Purpose:** Browse, inspect, replay, or delete stored codes.

### Steps

1. Menu → **2.View Code** → **SELECT**
2. Choose frequency (433 / 315) — same UI as sniff.
3. List view opens:
   ```
   433MHz 1/14
   Sel Info L=Back
   ```
4. Use **UP / DOWN** to change the selected code.
5. Press **SELECT** to open the **Detail view**:
   ```
   C:A1B2C3
   P1 B24 R4 ASK
   ```
   *(Code / Protocol / Bits / RSSI / Modulation)*
6. In Detail view:
   | Button | Action |
   |---|---|
   | **SELECT** | Replay this code |
   | **BACK** | **Delete** this code ⚠️ |
   | **LEFT** | Return to list |
7. From List view:
   - **LEFT** or **BACK** → return to main menu

### Replay notes
- Replay transmits the code once using the matching TX module.
- You should see `Sent!` on the LCD.
- If the target device does **not** respond, either the protocol, bit
  length, or pulse timing differs from the original (common with rolling
  codes).

### Pagination
- One code per screen (not packed), because each entry's detail is 16
  chars wide.
- Position indicator shows `current/total`.

---

## 3. Jam

**Purpose:** Transmit continuous OOK noise on **both** 315 and 433 MHz
simultaneously.

### Steps

1. Menu → **3.Jam** → **SELECT**
2. Warning screen appears for 1.5 s:
   ```
   Jamming 433+315
   L=Stop
   ```
3. Backlight turns off (power saving), then jamming starts.
4. Press **LEFT** to stop.

### ⚠️ Safety
- **Do not use outside a Faraday enclosure** unless you own everything
  nearby.
- Jamming may cause **permanent interference** with emergency devices —
  this can be a criminal offense.
- The device keeps transmitting until you press **LEFT**, or power is
  removed.
- The transmitter may become **warm** during extended jamming. Limit to
  short tests.

### What it does
- Rapidly toggles both TX pins at ~10 kHz.
- This occupies the band and blocks nearby receivers from decoding
  legitimate signals.

---

## 4. Brute-Force

**Purpose:** Iterate through all possible codes for a given bit length
and transmit each one.

### Steps

1. Menu → **4.Brute-Force** → **SELECT**
2. Choose frequency (433 / 315)
3. Choose bit length:
   ```
   Brute Bit Length
   >12b  16b(~30m)
   ```
   | Bits | Codes | Est. time |
   |---|---|---|
   | 12 | 4,096 | ~10 s |
   | 16 | 65,536 | ~30 min |
4. Press **SELECT** to start.
   ```
   Brute 433MHz 12b
   L=Stop
   ```
5. Progress updates every 32 codes.
6. Press **LEFT** to stop at any time.

### ⚠️ Legal warning
Brute-forcing is only legal on **your own device**. Attempting to brute-
force someone else's device is a criminal offense.

### Performance
- ~400 codes/sec (limited by `RCSwitch::send()` at protocol 1)
- The device **cannot** detect success — you must watch the target
  (e.g., does the door open?).
- For real cracking, you would need an SDR + proper protocol analysis
  (see **Universal Radio Hacker**).

---

## 5. Clear Data

**Purpose:** Wipe both `codes_433.csv` and `codes_315.csv`.

### Steps

1. Menu → **5.Clear Data** → **SELECT**
2. Confirmation prompt appears:
   ```
   Clear all?
   Sel=Y  Back=N
   ```
3. **SELECT** → files are truncated (empty). **BACK** or **LEFT** → cancel.

> This does **not** delete any custom files you added to the SD card.

---

## 6. Shutdown

**Purpose:** Safely halt all activity before you unplug the device.

### Steps

1. Menu → **6.Shutdown** → **SELECT**
2. Message appears:
   ```
   Shutting down
   ```
3. All receivers and transmitters are disabled. The SD card is flushed
   and released.
4. Final prompt appears:
   ```
   Safe Unplug
   Power off now
   ```
5. Backlight turns off. **You may now unplug the power.**

> ⚠️ The device **cannot physically cut its own power** — the prompt is
> a request for you to do so. Press **RESET** or cycle power to reboot.

> 💡 If you want hardware auto-power-off, add a **5V relay** or **P-MOSFET**
> controlled by a spare pin (e.g., A2) that cuts the main supply.

---

## SD Card Files

After a few captures, the SD card will contain:

```
/
├── codes_433.csv
└── codes_315.csv
```

### Format
Comma-separated, one line per capture:
```
<code_hex>,<protocol>,<bits>,<rssi>,<modulation>
```

Example `codes_433.csv`:
```
A1B2C3,1,24,4,ASK
F0E1D2,2,12,3,ASK
0000FF,1,24,0,ASK
```

### Opening on a PC
- **Linux / macOS:** open with Numbers / LibreOffice / `column -t -s,`
- **Windows:** rename to `.txt` or open directly with Excel (use comma
  as separator)
- Optional: convert to JSON for further analysis:
  ```bash
  awk -F, '{printf "{\"code\":\"%s\",\"proto\":%d,\"bits\":%d,\"rssi\":%d}\n",$1,$2,$3,$4}' codes_433.csv
  ```

---

## Tips & Best Practices

### Capture quality
- **Add antennas.** 17.3 cm for 433 MHz, 23.8 cm for 315 MHz (λ/4).
- Keep receivers away from USB cables and LCD ribbon (noise sources).
- Hold the device **within 1–5 m** of the target when first learning.
- Press the remote **3–5 times** to capture a full burst; RCSwitch often
  receives only on repetition.

### Storage
- Empty the SD card every few hundred captures, or codes accumulate.
- To archive, copy the CSV off the SD card and clear the file.

### Power
- **Do not power from a 3.3V pin** — Nano's onboard regulator is fine,
  but ASK receivers are picky about voltage. Use USB or a clean 5V source.
- If jamming for long periods, add a **heatsink** to the TX modules.

### Legal hygiene
- Keep a **lab notebook** with dates, times, and device ownership
  evidence for every test.
- Do all captures of third-party devices inside a **Faraday bag**.
- For research, submit findings through **responsible disclosure**.

---

## Troubleshooting

| Symptom | Possible cause | Fix |
|---|---|---|
| `SD Init Fail` | Wiring, wrong CS pin, bad card | Check D10, reformat FAT32, try a smaller card |
| LCD shows blank blocks | I2C address wrong | Change `0x27` → `0x3F` in the sketch |
| No codes captured | No antenna, too far, wrong band | Attach antenna, move closer, verify frequency |
| Every capture has `RSSI:0` | No RXB6 AGC pin | Use RXB6, or invert the `readRSSI()` formula |
| Replay doesn't work | Wrong protocol, rolling code | Verify target uses fixed code; re-capture |
| Brute-force freezes | Transmitting too fast | Lower loop rate (add `delayMicroseconds`) |
| Device restarts randomly | Brown-out (USB insufficient) | Use a dedicated 5V supply, add 100 µF cap |
| Garbage in CSV | Noise from adjacent bands | Use a band-pass filter, add a shielding can |
| `Read Err` in View | SD file corrupted | Reformat card, avoid ejecting during writes |

### Serial debugging (optional)
Serial is disabled by default to save **~128 B SRAM**. To re-enable:
1. Add `Serial.begin(115200);` in `setup()`
2. Uncomment `#define DEBUG` blocks (not included by default)
3. Note: you will lose some SRAM headroom

---

## FAQ

**Q: Can I sniff both 315 and 433 at the same time?**
A: Not truly simultaneously with these receivers. The current firmware
picks one band per session. For real dual-band capture, upgrade to
**CC1101**.

**Q: Why does RSSI always show 0?**
A: Most cheap ASK modules (XY-MK-5V) have no AGC pin. Only RXB6 and
similar superheterodyne receivers provide it.

**Q: Can I clone a car key fob?**
A: **No.** Modern cars use **rolling codes** (KeeLoq, HITAG, etc.). This
device only handles **fixed-code** OOK protocols (PT2262, EV1527). See
[protocols.md](protocols.md).

**Q: How far can it transmit?**
A: With a matched antenna and clear line of sight, ~10–30 m at 433 MHz.
Walls, metal, and other transmitters reduce this significantly.

**Q: Is this legal in my country?**
A: **Check your local law.** Sniffing and jamming are illegal in most
countries. See [DISCLAIMER.md](../DISCLAIMER.md).

---

**Happy sniffing — responsibly.** 🛰️
