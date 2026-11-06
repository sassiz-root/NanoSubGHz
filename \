# Sub-GHz OOK Protocols — Technical Reference

Understanding the encoding schemes used by consumer 315/433 MHz remote
controls, key fobs, and sensors.

This document covers the two most common fixed-code families:
**PT2262/PT2260/PT2264** and **EV1527/HS1527**, plus a comparison to
rolling-code systems.

---

## 1. Overview of Sub-GHz ASK/OOK

**ASK** (Amplitude Shift Keying) and its simpler cousin **OOK**
(On-Off Keying) modulate a carrier by switching it on/off:

```
Data:     1  0  1  1  0  1  0
Carrier:  █  _  █  █  _  █  _   (on=1, off=0)
```

A receiver sees the on/off pattern and reconstructs bits. **No clock is
transmitted** — the receiver must infer timing from the pulse widths.

**Common frequencies:**
- **315 MHz** (North America, Japan, some Asia)
- **433.92 MHz** (Europe, Iran, most of the world)
- **868 MHz** (EU, short range devices)
- **915 MHz** (Americas, ISM band)

**Timing:**
- OOK pulse widths are typically **200–500 µs**
- A frame is usually **12 to 32 bits** (plus sync)
- Full messages repeat every **20–100 ms** for reliability

---

## 2. PT2262 / PT2260 / PT2264 (Princeton / "Classic")

The **PT2262** is a low-cost encoder from Princeton Technology (later
cloned by many manufacturers: EV1527, HS1527, etc.). It is the
**dominant** family in garage door openers, cheap remotes, and wireless
doorbells worldwide.

### 2.1 Frame Structure

A PT2262 frame is:

```
[Sync] [Bit1] [Bit2] ... [Bit12..24] [Sync?]
```

Each frame contains **8 to 24 data bits** (typically 12 for 12-bit
encoders, 24 for 24-bit encoders).

### 2.2 Encoding (Ternary)

Each bit is encoded as a **pair of pulses** with different duty cycles:

| Symbol | Meaning | Timing |
|---|---|---|
| **`0`** (short-short) | Low bit | ▄▄ `_` ▄▄ `_` |
| **`1`** (long-short) | High bit | ▄▄▄▄ `_` ▄▄ `_` |
| **`f`** (short-long) | **Floating / tri-state** | ▄▄ `_` ▄▄▄▄ `_` |

Where `▄▄` ≈ **α** (short pulse), `▄▄▄▄` ≈ **3α** (long pulse), and the
gap is always ≈ **α**.

Typical **α ≈ 350 µs** for 433 MHz remotes (varies 150–500 µs).

### 2.3 Sync Pulse

Between frames, a **long sync pulse** appears: `▄` + long gap ≈ **31α**
(or 32α in some chips). The sync marks "start of new frame".

### 2.4 Address vs Data

On PT2262-based systems:

- **First N bits** = **address** (fixed per remote; the "house code")
- **Last M bits** = **data** (buttons / state)

For a 12-bit remote:
- 8 address bits (DIP switches) + 4 data bits (A/B/C/D buttons)
- Common split: **10 addr + 2 data**, or **8+4**

For a 24-bit remote:
- 20 address + 4 data (typical)

The **address is settable** via DIP switches (2^8 = 256 combos). If you
buy a second remote for the same door, you match its DIP switches.

### 2.5 Encoding Example (12-bit, address = `10110010`, button A pressed)

```
Original bits:  1 0 1 1 0 0 1 0 | 1 0 0 0   (last 4 = data, A = 1)
Pulse pattern:  1 0 1 1 0 0 1 0 1 0 0 0
After ternary:  [long-short][short-short][long-short]...
Hex code:       0xB2 8   (varies with bit order)
```

The exact hex value depends on bit order (MSB-first vs LSB-first) and
which bits are sent first. **RCSwitch reports the raw code**, so treat
the hex as opaque — just replay the same value.

### 2.6 RCSwitch Protocol Number

RCSwitch identifies PT2262 with **protocols 1 and 2**, differing in
bit ordering. Both decode to the same pulse train, so replaying with the
same protocol works fine.

### 2.7 Tri-State (`f`)

Some remotes use all three symbols (`0`, `1`, `f`). RCSwitch does not
natively support tri-state — it will decode `f` as either `0` or `1`.
**This means raw replay may fail.** You must re-transmit the exact pulse
train, which requires a custom encoder.

> 💡 Flipper Zero supports tri-state via its `Princeton` protocol
> variant. This project does **not** currently.

---

## 3. EV1527 / HS1527 (Learning Code)

**EV1527** is a **24-bit learning code** IC. Unlike PT2262, it is
**not DIP-switch configurable** — each chip has a **factory-set unique
20-bit serial number**. The last 4 bits are the button state.

### 3.1 Frame Structure

```
[20-bit ID] [4-bit button]   → 24 bits total
```

Example:
```
ID:     0xA1B2C    (20 bits)
Button: 0x1        (button A)
Frame:  0xA1B2C1   (24 bits, hex)
```

### 3.2 Encoding Timing

EV1527 uses the same **ternary pulse pair** as PT2262:

| Symbol | Timing |
|---|---|
| **0** | short-short |
| **1** | long-short |
| **Sync** | long gap (≈ 31α) |

**Same as PT2262** — only the bit count and structure differ.

Typical α ≈ **250–350 µs**.

### 3.3 Learning / Pairing

EV1527-based receivers use **"learning mode"**:

1. Hold the receiver's **pair button** for 3 s
2. Press the remote once
3. Receiver stores the 20-bit ID in EEPROM
4. Now the remote opens the door

**Up to 15–20 remotes** can be learned per receiver.

### 3.4 Security Implication

Since IDs are **fixed** and stored on the receiver, **replaying a captured
code works** — as long as you capture all bits correctly. This is why
these systems are considered insecure for anything sensitive.

### 3.5 RCSwitch Protocol

EV1527 decodes as RCSwitch **protocol 1** (same ternary timing). The hex
code from `getReceivedValue()` is the full 24-bit frame.

### 3.6 Clones

Many Chinese ICs are EV1527 clones:
- **HS1527** (Hopesun)
- **SC5262** (similar but 12-bit)
- **PT4303** (compatible)
- **XC4388** (compatible)

They all share timing, so they interoperate.

---

## 4. Comparison: PT2262 vs EV1527

| Feature | PT2262 | EV1527 |
|---|---|---|
| **Bits per frame** | 12 or 24 | 24 |
| **Address source** | DIP switches | Factory ID |
| **Address bits** | 8–20 | 20 |
| **Button bits** | 4 | 4 |
| **Learning** | None (DIP match) | Required (pair button) |
| **Clonable by replay** | Yes | Yes |
| **RCSwitch protocol** | 1, 2 | 1 |
| **Encryption** | None | None |

---

## 5. Rolling Code Systems (⚠️ Not Supported)

Modern remotes (cars, gates from 2000+, many new doorbells) use
**rolling codes** — the transmitted value **changes every press**.

### How it works (simplified, KeeLoq)

1. **Both remote and receiver** share a **secret key** (burned at factory)
2. Remote **increments a counter** each press
3. Remote computes **encrypted counter** = `E(key, counter)` and transmits
4. Receiver decrypts, compares to its expected next counter
5. If the counter is within **±16** of expected → accept, update counter
6. If out of range → reject (replay protection)

### Why this defeats capture-replay

- **Every press is a new code** — replay the same one, receiver rejects
- Attacker must know the **secret key** (128-bit AES for modern systems)
- Even a **captured pair** (previous + current) doesn't reveal the key
  without breaking the cipher

### Systems affected
- **KeeLoq** (Microchip HCS301, HCS200, HCS360)
- **HITAG 2** / **HITAG S** (NXP)
- **AES-128** (newer remotes)
- **Somfy** rolling code (older home automation)
- **Chamberlain Security+ 2.0**

### What you can do
**Nothing with this hardware.** Cracking rolling codes requires:
- **SDR** (HackRF, RTL-SDR + custom firmware, or Flipper Zero)
- **Side-channel analysis** (power analysis, electromagnetic)
- **Hours to weeks** of compute for vulnerable schemes

> ⚠️ **Attempting to crack a rolling code system you don't own is a
> serious criminal offense** (computer fraud, theft, unauthorized
> access). Don't.

---

## 6. Other Protocols You May Encounter

| Protocol | Bits | Notes | RCSwitch? |
|---|---|---|---|
| **PT2262** | 12/24 | Ternary tri-state | Partial (proto 1, 2) |
| **PT2260** | 12 | Simplified, no tri-state | Yes (proto 1) |
| **EV1527** | 24 | Factory ID, learnable | Yes (proto 1) |
| **HS1527** | 24 | EV1527 clone | Yes |
| **SC5262** | 12 | PT2262 clone | Yes |
| **HCS301** | 66 | Rolling (KeeLoq) | ❌ No |
| **Nexus / Intertechno** | 32 | Tri-state, similar to PT2262 | Partial |
| **Somfy RTS** | 56+ | Rolling + encryption | ❌ No |
| **CAME** | variable | Rolling (older) | ❌ No |
| **Oregon Scientific** | 32–64 | Weather sensors, FSK | ❌ No |
| **Bresser / LaCrosse** | 40–64 | Weather, FSK | ❌ No |

---

## 7. Practical Detection with RCSwitch

RCSwitch auto-detects protocols by matching pulse-width ratios against
known patterns.

### How `RCSwitch::available()` works
1. An interrupt records **pulse durations** into a buffer
2. When a **long gap** is detected, decoding starts
3. For each of protocols 1–12:
   - The bit buffer is split using protocol-specific timing
   - Bits are classified as `0` / `1` / `f` (or rejected)
   - If **all bits decode** with consistent timing → success
4. `getReceivedProtocol()` returns the matching protocol number

### Why false positives happen
With cheap ASK receivers, **noise** often mimics pulse trains:

- **Random bit patterns** may pass timing checks with ~12 bits
- **Interference** from adjacent devices (Wi-Fi, BLE, switching PSUs)
- **Reflections** of the same signal (multipath) may decode twice

### How to filter noise

**Software-side (do it yourself after capture):**
- Discard codes shorter than 12 bits
- Discard codes with `RSSI:0` (no AGC signal)
- Discard codes that repeat **once** — real remotes repeat 5–20 times
- Merge identical consecutive codes into one entry

**Hardware-side:**
- Use a **band-pass filter** (e.g., SAW filter for 433.92 MHz)
- Add a **shielding can** to the receiver
- Keep away from **Wi-Fi routers and switched-mode PSUs**

**RCSwitch tuning:**
- `#define RCSWITCH_MAX_CHANGES` (default 67) — reduce to filter noise
- `RCSwitch::setReceiveTolerance(60)` — tune sensitivity (50–80)

---

## 8. Replay Accuracy — Why It Sometimes Fails

Even for **fixed-code** systems, replay can fail:

| Cause | Explanation | Fix |
|---|---|---|
| **Tri-state bits** | PT2262 `f` decoded as `0`/`1` | Custom encoder needed |
| **Bit order** | MSB vs LSB differs | Try both protocols |
| **Timing mismatch** | Original uses 350 µs, replay uses 400 µs | Adjust `setProtocol()` timing or use raw pulse replay |
| **Incomplete capture** | Some bits missing (noise) | Capture again, close range |
| **Antenna mismatch** | Wrong frequency or length | Verify 433 vs 315 |
| **Weak TX** | FS1000A only outputs ~10 dBm | Move closer, better antenna |
| **Rolling code** | Every press is new | Not supported |

### Debugging failed replay

1. **Capture the same remote 3 times** → compare hex values
   - Same value each time → fixed code (good)
   - Different values → rolling code (unsupported)
2. **Check bits count** → 12, 24, or 32?
3. **Try protocol 1 and 2** — sometimes one works when the other doesn't
4. **Shorten distance** to 10 cm from the target
5. **Check with an SDR** (RTL-SDR is $25) to see what you're actually
   transmitting

---

## 9. Building an SDR Log (Recommended)

For serious research, use an **RTL-SDR** with:
- **[Universal Radio Hacker (URH)](https://github.com/jopohl/urh)** — GUI
  for protocol analysis
- **[rtl_433](https://github.com/merbanan/rtl_433)** — CLI decoder for
  dozens of commercial sensors
- **[inspectrum](https://github.com/miek/inspectrum)** — waveform viewer

Capture once with URH, inspect pulses visually, then replay with NanoSubGHz
using the exact bit pattern. This closes the loop between "captured hex"
and "actual pulse train".

---

## 10. Security Recommendations (for Manufacturers)

If you're **designing** a wireless product and reading this to learn what
not to do:

1. **Never use PT2262 or EV1527** in new products
2. **Use a rolling-code scheme** with a real cipher (AES-128, or a
   vetted lightweight cipher like **PRESENT**, **SIMON**, **SPECK**)
3. **Authenticate both directions** (challenge-response)
4. **Use frequency hopping** or **DSSS** where possible
5. **Add a nonce/timestamp** to prevent replay
6. **Consider BLE or UWB** with proper pairing for proximity use cases
7. **Publish a security contact** and be ready for disclosure

---

## 11. References

### Academic / Standard
- **FCC Part 15.231** — Periodic operation in the 40.66–40.70 MHz and
  above 70 MHz bands
- **EN 300 220** — ETSI standard for SRD in 25–1000 MHz
- **NIST SP 800-38** — Block cipher modes (for modern designs)

### Community
- **RCSwitch** — [github.com/sui77/rc-switch](https://github.com/sui77/rc-switch)
- **Universal Radio Hacker** — [github.com/jopohl/urh](https://github.com/jopohl/urh)
- **rtl_433** — [github.com/merbanan/rtl_433](https://github.com/merbanan/rtl_433)
- **Flipper Zero Sub-GHz docs** — [docs.flipper.net](https://docs.flipper.net)
- **Proxmark3** — [github.com/RfidResearchGroup/proxmark3](https://github.com/RfidResearchGroup/proxmark3)

### Datasheets
- **PT2262** — Princeton Technology
- **EV1527** — EV Microelectronics
- **RXB6** — [Datasheet PDFs online]

---

## ⚠️ Final Reminder

Everything in this document is **public technical knowledge**, intended
to explain **why** legacy systems are insecure. **Do not use it to
attack devices you do not own.** See [DISCLAIMER.md](../DISCLAIMER.md).

**Be a hacker, not a criminal.**

---

*Last updated: 2025 — Sassiz.Root*
