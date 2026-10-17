# Magic Mayer Home-Made Pedal: Plan

Recreate three pedals, **Tube Screamer (TS808/TS9)**, **Klon Centaur** and **Keeley Katana clean boost**, in two ways and compare them:

- **Branch A, analog (no microcontroller):** real circuits made of op-amps, diodes, resistors and capacitors. Each pedal is its own box with its own true-bypass footswitch.
- **Branch B, digital (ESP32):** one box. The guitar signal is digitized, `pedal.h` runs all three effects in code, and each effect has its **real front-panel knobs** plus its own footswitch.

The software side is already built: a **desktop tuner** and a **command-line simulator** run the exact `pedal.h` the ESP32 will run,
and compare it against a **recording of a real TS9**. Tune the sound on the PC, then flash the result to the pedal.

Goals: learn electronics (analog front end, filters, clipping) and DSP (filters in code), end with usable pedals, and tune the digital models until they sound like the analog originals.

> Only the **TS** is required in Branch A, as the reference. The analog Klon and Katana are optional builds (A2, A3).

---

## Contents

1. [Overview and timeline](#1-overview-and-timeline)
2. [Phase 0: Software, simulate and tune before buying](#2-phase-0-software-simulate-and-tune-before-buying)
3. [Controls: real front panels](#3-controls-real-front-panels)
4. [Shared parts (both branches)](#4-shared-parts-both-branches)
5. [Branch A: analog, no microcontroller](#5-branch-a-analog-no-microcontroller)
6. [Branch B: digital, ESP32](#6-branch-b-digital-esp32)
7. [Shopping list by shop](#7-shopping-list-by-shop)
8. [Build steps](#8-build-steps)
9. [Firmware](#9-firmware)
10. [Compare analog vs digital](#10-compare-analog-vs-digital)
11. [Permanent build](#11-permanent-build)
12. [Risks, gotchas, later ideas](#12-risks-gotchas-later-ideas)

---

## 1. Overview and timeline

```
Branch A (analog):   Guitar → [TS808] → [Klon]* → [Katana]* → Amp        * optional
                              3PDT      3PDT      3PDT        (one true-bypass switch per box)

Branch B (digital):  Guitar → TL072 buffer → PCM1808 → ESP32 (pedal.h) → PCM5102A → master vol → Amp
                              1 MΩ input     ADC       TS/Klon/Katana    DAC
                                                        ↑ 7 pots + pull switch + 3 footswitches

PC (done):           recording → tuner / sim (same pedal.h) → presets/*.cfg → baked into the firmware

Hybrid (final idea): Guitar → [analog TS808] → [ESP32: Klon + Katana] → Amp
```

| Phase | What | Branch | Time | Status |
|---|---|---|---|---|
| 0 | Software: tuner, simulator, real TS9 recording | – | – | ✅ done |
| 0b | Tune presets in the tuner (A/B against the real TS9) | – | evenings, open-ended | ⬜ |
| 1 | Order parts | both | 1 evening + 1–3 weeks shipping | ⬜ |
| 2 | Build analog TS kit | A | 1 evening (3–4 h) | ⬜ |
| 2b | Build analog Klon / Katana (optional) | A | 1 evening each (Klon: 4–6 h) | ⬜ |
| 3 | Breadboard the digital pedal | B | 1 weekend (6–10 h) | ⬜ |
| 4 | Firmware: audio loop, knobs, footswitches, config | B | 2–3 evenings | ⬜ |
| 5 | Compare + tune on hardware | both | open-ended | ⬜ |
| 6 | Permanent digital build (enclosure) | B | 1–2 weekends | ⬜ |

Order of work: **tune in software now → order parts → Musikding parts arrive first (≈1 week) → build the analog TS while the AliExpress parts (2–3 weeks) are in transit.**

---

## 2. Phase 0: Software, simulate and tune before buying

### 2a. What's built
| File | What |
|---|---|
| `pedal.h` | The three effects, **shared by every tool and the ESP32**. Every setting is named and loadable from config text |
| `tuner` | Desktop app: real-time playback through `pedal.h`, front-panel knobs + circuit internals, Dry / Model / Real-pedal A/B (loudness matched), amp + cab listening aid, live plots, presets |
| `sim` | Command line: renders every effect combination to `renders/`, with self-checks |
| `recordings/` | Real Telecaster DI + the same signal through a **real Ibanez TS9** (max drive, tone and level at noon), from GuitarML |
| `presets/*.cfg` | Saved settings: `default`, `ts9_max_drive` (same knobs as the recording) |

```bash
make deps && make recordings && make
./tuner                                     # or drag any dry guitar WAV onto the window
./sim [guitar.wav] [presets/NAME.cfg]
```

### 2b. Tuning workflow
- [ ] Load the `ts9_max_drive` preset, press **2 / 3** to compare Model vs Real pedal, and lower the **tone difference** score using the TS circuit internals (mid hump, tone corner, gain)
- [ ] Bring the **compression** close to the real TS9: its output loudness varies ~6 dB while the input varies ~15 dB
- [ ] Check the harmonics plot: a symmetric TS clipper gives mostly odd harmonics
- [ ] Record **your own guitar** dry (any audio interface, 16/24-bit WAV) and drag it in: pickups change everything
- [ ] Save one preset per sound (e.g. `ts-lead`, `klon-push`, `katana-clean`)
- [ ] Turn **Amp + cab** off once in a while: the ESP32 will output the pedal only, so make sure it doesn't rely on the amp sim to sound good

### 2c. Analog circuit simulation (free, for learning the TS circuit)
| Tool | Use |
|---|---|
| **Falstad** (falstad.com/circuit, browser) | Quick, visual: watch the signal and the clipping live |
| **KiCad + ngspice** (`sudo dnf install kicad`) | Frequency-response (AC) plots; the same tool you'll use for a PCB |
| LTspice (Windows/Mac, or Wine) | Many existing Tube Screamer `.asc` files online |

- [ ] Draw the TS808 clipping stage: op-amp, 4.7 kΩ + 47 nF to Vref, 51 kΩ + 500 kΩ drive pot with 51 pF in the feedback loop, two 1N914 in the feedback loop
- [ ] Find the ~720 Hz high-pass corner: f = 1 / (2π · 4.7 kΩ · 47 nF) ≈ 720 Hz. That's `tsHpHz` in `pedal.h`
- [ ] Check the gain formula behind the DRIVE knob: 1 + (51 kΩ + R_drive) / 4.7 kΩ ≈ 12× to 118×. That's `applyPanel()` in `pedal.h`

### 2d. Controls simulation (optional)
- [ ] **Wokwi** (browser ESP32): pots, buttons, LEDs. It can't simulate I2S audio.

---

## 3. Controls: real front panels

Both the tuner and the pedal use the real front panels, 0–10 like the real knobs. `Pedal::applyPanel()` turns knob positions into circuit values,
so the tuner's sliders and the hardware pots go through **the same code**.

| Pedal | Real control | Config key | What it sets in the model |
|---|---|---|---|
| TS808 / TS9 | DRIVE | `tsDriveKnob` | `tsDrive` = 1 + (51k + 500k audio-taper pot) / 4.7k → **11.9× … 118×** (the real TS808 formula) |
| | TONE | `tsToneKnob` | `tsToneHz` 500 Hz … 6 kHz (approximation) |
| | LEVEL | `tsLevelKnob` | `tsLevel` off, −24 … +6 dB |
| Klon Centaur | GAIN (dual-gang) | `klGainKnob` | `klGain` 1 … 60× **and** `klBlend` clean → dirty, together like the real Klon |
| | TREBLE | `klTrebleKnob` | `klTreble` 1/3 … 3, flat at noon |
| | OUTPUT | `klOutputKnob` | `klLevel` off, −24 … +12 dB |
| Keeley Katana | VOLUME | `kaVolumeKnob` | `kaBoost` off, −20 … +35 dB |
| | push/pull | `kaDirty` | dirty boost: extra gain into soft clipping (approximation) |
| all | footswitch | `tsOn`, `klonOn`, `katanaOn` | effect on/off |

**Circuit internals** (not on any real front panel, and not on the hardware either): `tsHpHz`, `klClip`, `klBpHz`, `klTrebleHz`, `kaBright`, `kaShelfHz`,
plus direct overrides of everything the knobs set. Tune these in the tuner and bake them into the firmware ([section 9](#9-firmware)).

Config files list knobs first and circuit values after: setting a knob re-derives its circuit values, and the lines after it override them.

---

## 4. Shared parts (both branches)

### Power
| Qty | Part | Notes |
|---|---|---|
| 1 | **9 V DC pedal power supply**, center-negative, 2.1 mm, ≥500 mA, **isolated outputs** preferred | Isolated outputs prevent ground-loop hum with several pedals on one supply. A non-isolated adapter + daisy chain is fine to start |
| 1 | Daisy-chain cable (2.1 mm, 3–5 plugs) | Only with a non-isolated supply |

### Cables
| Qty | Part |
|---|---|
| 3–4 | 15–30 cm patch cables (6.35 mm mono) |
| 1 | Guitar cable |

### Tools
| Qty | Part | Notes |
|---|---|---|
| 1 | Multimeter | **Essential.** DC volts (bias), continuity (wiring), AC volts (level matching) |
| 1 | Temperature-controlled soldering iron (Pinecil / TS101 / any 60 W station) | |
| 1 | Solder, 0.6–0.8 mm, leaded 60/40 or 63/37 (easier) or lead-free | |
| 1 | Desoldering pump + desoldering wick | |
| 1 | Side cutters (flush) + wire strippers | |
| 1 | Helping hands / PCB holder | |
| 1 | Solid-core hookup wire (22–24 AWG), several colors | |
| 1 | Heat-shrink tubing assortment | |
| 1 | Step drill bit + center punch | For the enclosures |
| 1 | Headphones or a small powered speaker | Testing without the amp |
| 1 | USB audio interface (if you don't have one) | To record your own dry guitar for the tuner, and to reamp |

---

## 5. Branch A: analog, no microcontroller

Each pedal is a separate box: **PCB + parts + enclosure + 3PDT true-bypass switch + jacks + DC jack + LED.**

> Buying a **complete kit** (PCB + all parts + drilled enclosure) is strongly recommended. It ships with its own exact parts list (BOM), which is authoritative.
> The lists below explain **what each part does** so you can follow the circuit. Values are from the well-known TS808 schematic (ElectroSmash analysis) from memory: check them against your kit's BOM.

### A0. Off-board hardware every analog pedal needs
| Qty per pedal | Part | Notes |
|---|---|---|
| 1 | **3PDT latching footswitch** | True bypass. **Latching** (click on/off), unlike Branch B's momentary switches |
| 2 | 6.35 mm **mono** jacks (open-frame, Switchcraft/Cliff style) | Input + output |
| 1 | 2.1 mm DC jack (panel, isolated) | Center-negative |
| 1 | LED (3 or 5 mm) + bezel | On/off indicator |
| 1 | LED resistor, 2.2–4.7 kΩ | Higher = dimmer |
| 1 | 3PDT LED daughterboard (optional) | Much easier footswitch wiring |
| 1 | Enclosure: **1590B** (TS, Katana) / **125B** or 1590BB (Klon) | Pre-drilled if offered |
| – | Knobs (one per pot) | |
| 1 | 9 V battery clip (optional) | |

### A1. Tube Screamer TS808 (required, the reference)

**Buy:** a TS808 kit from **Musikding.de** (or any TS808 PCB + parts).

| Stage | Parts | Job |
|---|---|---|
| **Power** | 1N4001 (polarity), 2× 10 kΩ divider + 47 µF → **Vref 4.5 V**, 100 µF supply filter | "Virtual ground" for the single-supply op-amp |
| **Bypass switching** (original only) | JFETs + transistors + flip-flop | **Kits replace this with the 3PDT** → skip |
| **Input buffer** | 2SC1815 (emitter follower), ~20 nF input cap, 510 kΩ bias resistors, 10 kΩ emitter resistor | High input impedance so the pickups aren't loaded |
| **Clipping stage** | **JRC4558D** (half A), **4.7 kΩ + 47 nF** to Vref (**720 Hz high-pass** = mid hump), **51 kΩ + 500 kΩ (A) DRIVE pot** in the feedback loop, **51 pF** across it (high roll-off), **2× 1N914/1N4148** antiparallel in the feedback loop | Gain ~12× to ~118×. Diodes in the feedback loop = the smooth TS character |
| **Tone + volume** | 1 kΩ + 220 nF fixed low-pass (≈723 Hz), **20 kΩ TONE pot**, 4558 (half B) active tone stage, 220 Ω, 1 kΩ, 220 nF, **100 kΩ (A) LEVEL pot** | Treble cut/boost and output level |
| **Output buffer** | 2SC1815 (emitter follower), **100 Ω** series (TS808; TS9 uses 470 Ω), 10 kΩ pull-down, 1 µF / 10 µF coupling | Low output impedance |

Pots: **DRIVE 500 kΩ (A/log)**, **TONE 20 kΩ (W/G taper, or B)**, **LEVEL 100 kΩ (A)**.
About 25 resistors, 15 capacitors, 1 op-amp (+ **DIP-8 socket** to swap op-amps: JRC4558 vs TL072 vs RC4558), 2–3 transistors, 3 diodes, 3 pots.

**Learning value: ⭐⭐⭐⭐⭐.** Every filter in `pedal.h`'s TS block corresponds to a part on this board.

### A2. Klon Centaur clone (optional)

**Buy:** a complete Klon clone kit: PedalPCB **"Kliché"**, Aion FX "Refractor", the General Guitar Gadgets replica kit, or Musikding's own.

| Part | Job |
|---|---|
| **Charge pump (TC1044SCPA / ICL7660 / LT1054)** | 9 V → ~18 V for more clean headroom |
| **2× TL072** (+ sockets) | Gain stage, summing stage, tone, output |
| **2× 1N34A germanium diodes** | Hard clipping to ground: the grit (`klClip` in code) |
| **Dual-gang 100 kΩ (B) GAIN pot** | One gang sets the gain, the other cross-fades clean ↔ dirty (`klGainKnob` → `klGain` + `klBlend`) |
| 10 kΩ (B) TREBLE pot | Active treble control (`klTrebleKnob`) |
| 10 kΩ (B) OUTPUT pot | Level (`klOutputKnob`) |
| ~60 passives | 4–6 h of soldering: the most complex board here |

**125B** enclosure.

### A3. Keeley Katana clone (optional)

**Buy:** a "Katana-style" boost PCB: community layouts on the **PedalPCB** forum ("Cleaver" boost) and from **Aion FX**. Check which Katana version (v1 or v2) it follows.
Key parts: **JFETs**, a **charge pump** for 18 V headroom, a VOLUME pot (**push/pull** for the dirty mode on versions that have it).
Smallest board, 1–2 h build, 1590B enclosure.

> No verified Katana schematic was found: use the BOM that comes with the PCB.

### Branch A cost (approximate)
| Build | Cost |
|---|---|
| TS808 kit + enclosure + hardware | €35–60 |
| Klon kit | €40–80 |
| Katana-style PCB + parts | €25–40 |

---

## 6. Branch B: digital, ESP32

One box: analog front end → converters → ESP32 → real front-panel knobs and three momentary footswitches.

### B1. Bill of materials

#### Brain + converters
| Ref | Qty | Part | Notes |
|---|---|---|---|
| U1 | 1 (+1 spare) | **ESP32 DevKit** (ESP32-WROOM-32, 38-pin) | Must break out every GPIO in [the pin plan](#pin-plan) |
| U2 | 1 (+1–2 spares) | **PCM1808 module** (ADC, input) | Needs MCLK from the ESP32 |
| U3 | 1 (+1–2 spares) | **PCM5102A module** (GY-PCM5102, DAC, output) | Set the jumpers on the back! |
| – | 1 (alt.) | Digilent **Pmod I2S2** | Replaces U2 + U3 with one board (DigiKey/Mouser) |

#### Input buffer
| Ref | Qty | Part | Value | Job |
|---|---|---|---|---|
| U4 | 1 (+4 spares) | **TL072** (DIP-8) + socket | – | Op-amp buffer |
| R1 | 1 | resistor | **1 MΩ** | Input pull-down (no pops) |
| C1 | 1 | **film** capacitor | **100 nF** | Input coupling (blocks DC) |
| R2 | 1 | resistor | **1 MΩ** | Bias to Vref **and the 1 MΩ input impedance** pickups need |
| R3, R4 | 2 | resistor | **100 kΩ** | Vref divider 9 V → 4.5 V |
| C2 | 1 | electrolytic | **47 µF / 16 V+** | Smooths Vref |
| C3 | 1 | electrolytic | **10 µF / 16 V+** | Output coupling to the PCM1808 |
| C4 | 1 | ceramic | **100 nF** | TL072 decoupling (at pin 8) |
| – | – | – | – | **Unused TL072 half:** pin 5 → Vref, pin 6 ↔ pin 7 (stops it oscillating) |

#### Output
| Ref | Qty | Part | Value | Job |
|---|---|---|---|---|
| RV8 | 1 | pot | **10 kΩ (A/log)** | Master volume. The PCM5102A puts out ~2.1 Vrms, too hot for an amp's input. Analog, not read by the ESP32 |
| R5 | 1 | resistor | 1 kΩ | Series output protection |

#### Power
| Ref | Qty | Part | Value | Job |
|---|---|---|---|---|
| J3 | 1 | 2.1 mm DC jack | center-negative | 9 V in |
| D1 | 1 | Schottky | **1N5817** | Reverse polarity protection |
| C5 | 1 | electrolytic | 100 µF / 16 V+ | 9 V bulk |
| U5 | 1 (+1–2 spares) | 5 V regulator module | **AMS1117-5.0** or **7805** | 9 V → 5 V for the ESP32 + converters |
| C6, C7 | 2 | electrolytic | 10 µF | Regulator in/out (if the module lacks them) |
| C8–C10 | 3 | ceramic | 100 nF | Decoupling at U1, U2, U3 |

The TL072 runs directly on 9 V (it needs ≥ 7 V). The ESP32's 3.3 V comes from its onboard regulator.

#### Front panel: the real pedals' controls
| Ref | Qty | Part | Value | Function |
|---|---|---|---|---|
| RV1 | 1 | pot | **10 kΩ linear (B)** | TS **DRIVE** |
| RV2 | 1 | pot | 10 kΩ B | TS **TONE** |
| RV3 | 1 | pot | 10 kΩ B | TS **LEVEL** |
| RV4 | 1 | pot | 10 kΩ B | Klon **GAIN** (one pot is enough: the code moves gain and blend together) |
| RV5 | 1 | pot | 10 kΩ B | Klon **TREBLE** |
| RV6 | 1 | pot | 10 kΩ B | Klon **OUTPUT** |
| RV7 | 1 | **push/pull pot** | 10 kΩ B, with built-in switch | Katana **VOLUME** + pull = **dirty boost** |
| C11–C17 | 7 | ceramic | 100 nF | Pot wiper → GND: less noise in the readings |
| SW1–SW3 | 3 | **momentary** switch | tactile (breadboard) → **soft-touch momentary footswitch** (final) | TS / Klon / Katana on-off. **Momentary, not latching**: the ESP32 keeps the state |
| LED1–3 | 3 | LED 3/5 mm | – | Effect on/off |
| R6–R8 | 3 | resistor | 1 kΩ (brighter) … 4.7 kΩ (dimmer) | LED current |

All pots are **linear (B)** even where the real pedal uses an audio taper: the taper is applied in code (`Pedal::logTaper`), so the hardware stays simple.

#### Connectors + prototyping
| Ref | Qty | Part |
|---|---|---|
| J1, J2 | 2 | 6.35 mm mono jacks (in, out) |
| – | 2 | 830-point breadboards |
| – | 1 pack each | Jumper wires M-M, M-F |
| – | 1 | Pin headers (solder onto the modules) |
| – | 2 | Stripboard / perfboard |
| – | 1 each | Resistor assortment + capacitor assortment |

#### Enclosure (buy **after** the breadboard version works)
| Qty | Part |
|---|---|
| 1 | **1590XX** or **125B** (landscape), more room than a 1590BB for 7 knobs + 3 footswitches |
| 7 | Knobs (one pull-friendly knob for RV7) |
| 3 | LED bezels |

```
 ┌──────────────────────────────────────────────┐
 │ (DRV)(TONE)(LVL)  (GAIN)(TRB)(OUT)  (VOL↕)   │   ↕ = push/pull
 │      ● TS             ● KLON         ● KAT   │
 │     [ FS ]           [ FS ]         [ FS ]   │
 │                                   (MASTER)   │   side or top: master volume, jacks, DC
 └──────────────────────────────────────────────┘
```

#### Expansion (optional, cheap now)
| Qty | Part | Unlocks |
|---|---|---|
| 1 | ESP32-**WROVER** (4–8 MB PSRAM) | Delay / reverb |
| 1–2 | SSD1306 0.96" OLED (I2C) | Show settings, preset names |
| 2 | EC11 rotary encoder with push | Edit circuit internals and presets on the pedal |
| 1–2 | Extra soft-touch footswitches | Preset switching, tap tempo |
| 1 | 6.35 mm **stereo** (TRS) jack | Expression pedal / headphone out |

### B2. Pin plan

| Function | ESP32 GPIO | Notes |
|---|---|---|
| I2S MCLK | 0 | 12.288 MHz (256 × 48 kHz). **Boot pin**: unplug while flashing if uploads fail |
| I2S BCK | 26 | shared by both converters |
| I2S WS / LRCK | 25 | shared |
| I2S data out → PCM5102A DIN | 22 | |
| I2S data in ← PCM1808 OUT | **19** | moved from 33 to free an analog pin |
| RV1 TS DRIVE | 32 | ADC1 |
| RV2 TS TONE | 33 | ADC1 |
| RV3 TS LEVEL | 34 | ADC1 (input only) |
| RV4 Klon GAIN | 35 | ADC1 (input only) |
| RV5 Klon TREBLE | 36 (VP) | ADC1 (input only) |
| RV6 Klon OUTPUT | 39 (VN) | ADC1 (input only) |
| RV7 Katana VOLUME | 4 | **ADC2: works only while Wi-Fi is off** (it always is in this firmware) |
| RV7 pull switch (Katana dirty) | 23 | to GND, internal pull-up |
| SW1 / SW2 / SW3 footswitches | 13 / 14 / 27 | to GND, internal pull-ups, 20–30 ms debounce |
| LED1 / LED2 / LED3 | 16 / 17 / 18 | through R6–R8 |
| Reserved: OLED I2C SDA / SCL | 21 / 5 | expansion |

Pot wiring: one end to **3.3 V** (never 5 V), the other to GND, wiper to the GPIO with C11–C17 to GND.

### Branch B cost (approximate)
| Part | Cost |
|---|---|
| Converter modules (incl. spares) | €10–20 |
| Passives, TL072, regulator, breadboards, wires | €20–35 |
| Front panel: 6 pots + 1 push/pull pot, buttons, LEDs | €15–25 |
| Jacks, DC jack, footswitches (final) | €15–25 |
| Enclosure + knobs (final) | €20–30 |
| Expansion (optional) | €15–25 |

---

## 7. Shopping list by shop

✅ = needed for that branch, ➖ = not needed.

### Musikding.de (pedal parts, ≈1 week to Denmark): **order first**
| Item | Qty | A | B |
|---|---|---|---|
| **TS808 kit** (complete, with enclosure if offered) | 1 | ✅ | ➖ |
| Klon kit (optional) | 1 | opt | ➖ |
| 3PDT latching footswitches (if not in the kits) | 1 per pedal | ✅ | ➖ |
| **Soft-touch momentary footswitches** | 3 (+2) | ➖ | ✅ |
| **10 kΩ (B) push/pull pot** | 1 (+1) | ➖ | ✅ |
| 6.35 mm mono jacks | 2 per pedal + 2 | ✅ | ✅ |
| 6.35 mm stereo jack | 1 | ➖ | opt |
| 2.1 mm DC jacks | 1 per box | ✅ | ✅ |
| **1590XX or 125B** enclosure | 1 (later) | ➖ | ✅ |
| Knobs, LED bezels | as needed | ✅ | ✅ |
| Spare TS pots: 500 kΩ A, 20 kΩ W/B, 100 kΩ A | 1 each | opt | ➖ |
| JRC4558D op-amps (to try swapping) | 2 | opt | ➖ |
| 1N914, 1N34A, LEDs as clipping diodes (to experiment) | a few | opt | ➖ |

### AliExpress / eBay.de (cheap, 1–3 weeks): **order on day 1**
| Item | Qty | A | B |
|---|---|---|---|
| PCM1808 module | 2–3 | ➖ | ✅ |
| PCM5102A module | 2–3 | ➖ | ✅ |
| ESP32 DevKit (spare) | 1 | ➖ | ✅ |
| ESP32-WROVER | 1 | ➖ | opt |
| SSD1306 OLED | 1–2 | ➖ | opt |
| EC11 encoders | 2 | ➖ | opt |

Faster EU alternatives: PCM5102A from Electrokit (SE) / Otronic (NL); **Pmod I2S2** from DigiKey/Mouser.

### Danish hobby shop (Let Elektronik / Elektronik-Lavpris / Ardustore / Elextra)
| Item | Qty | A | B |
|---|---|---|---|
| TL072 (DIP-8) + DIP-8 sockets | 5 + 5 | opt | ✅ |
| AMS1117-5.0 / 7805 regulator module | 2–3 | ➖ | ✅ |
| 1N5817 diodes | 5 | ➖ | ✅ |
| Resistor assortment (E12, ¼ W, incl. 1 MΩ, 100 kΩ, 1 kΩ, 4.7 kΩ) | 1 | ✅ | ✅ |
| Capacitor assortment: ceramic 100 nF (≥ 20), electrolytic 10/47/100 µF | 1 each | ✅ | ✅ |
| Film capacitors 100 nF | 5 | ➖ | ✅ |
| 830-point breadboards | 2 | opt | ✅ |
| Jumper wires M-M / M-F, pin headers | 1 pack each | ➖ | ✅ |
| Tactile buttons | 5 | ➖ | ✅ |
| LEDs (3/5 mm) | 10 | ✅ | ✅ |
| **B10K linear pots** | **6 (+2 spare)** | ➖ | ✅ |
| A10K log pot (master volume) | 1–2 | ➖ | ✅ |
| Stripboard / perfboard | 2 | ➖ | ✅ |
| Hookup wire, heat shrink | 1 each | ✅ | ✅ |

### Thomann (if you don't have them)
| Item | Qty | A | B |
|---|---|---|---|
| 9 V isolated pedal power supply | 1 | ✅ | ✅ |
| Patch cables | 3–4 | ✅ | ✅ |
| USB audio interface (e.g. a 2-input model) | 1 | opt | opt |

### DigiKey / Mouser / Farnell (fallback)
Free shipping above a threshold. Useful for: TL072, TC1044SCPA, JRC4558D, film caps, Pmod I2S2, push/pull pots.

---

## 8. Build steps

### 8A. Branch A: analog TS (1 evening)
1. [ ] Sort the parts; check every resistor with the multimeter
2. [ ] Solder **lowest parts first**: resistors → diodes (**stripe direction**) → IC socket → small caps → transistors → electrolytics (**polarity**) → pots
3. [ ] Off-board wiring: jacks, DC jack, 3PDT, LED
4. [ ] **Before inserting the op-amp:** power up and measure 9 V at pin 8 and **4.5 V at Vref**
5. [ ] Insert the 4558 (pin 1 dot!) and play
6. [ ] Find each stage from the [A1 table](#a1-tube-screamer-ts808-required-the-reference) on the board
7. [ ] Experiments: swap op-amps, swap 1N914 for 1N34A or LEDs, change C3 47 nF → 100 nF. Try the same changes in the tuner's circuit internals

### 8B. Branch A: Klon / Katana (optional)
Same order. Klon: **measure ~17–18 V at the charge pump output** before inserting the TL072s.

### 8C. Branch B: breadboard (1 weekend)
Build in this order and **test each step before moving on**:

1. [ ] **Power:** 9 V → D1 → C5 → U5 → 5 V rail. **Measure 5.0 V** before connecting anything
2. [ ] **ESP32 alone:** flash a blink sketch. Never power it from USB and the 5 V rail at the same time
3. [ ] **Output (DAC):** U3 with its jumpers set, play a **440 Hz sine**, listen through RV8 on headphones
4. [ ] **Input (ADC):** U2 with MCLK, **passthrough** (ADC → DAC). Test with phone audio first
5. [ ] **Buffer:** TL072 stage, then the guitar. Check: not dull, no hum, no distortion on hard strums
6. [ ] **Front panel:** RV1–RV7 + pull switch; print 0–10 knob values over serial and check each knob's full travel
7. [ ] **Footswitches + LEDs:** SW1–3, LED1–3
8. [ ] **Firmware with `pedal.h`:** [section 9](#9-firmware)

#### I2S wiring (the ESP32 is the clock master)
| ESP32 | PCM1808 (U2) | PCM5102A (U3) |
|---|---|---|
| GPIO0 (MCLK) | SCK | – (board SCK → **GND** = internal PLL) |
| GPIO26 (BCK) | BCK | BCK |
| GPIO25 (WS / LRCK) | LRC | LCK |
| **GPIO19** (data in) | OUT | – |
| GPIO22 (data out) | – | DIN |
| 5 V / GND | VCC (5 V), VDD (3.3 V) / GND | VIN / GND |

- **PCM5102A jumpers (back side):** FLT = L, DEMP = L, **XSMT = H**, FMT = L. If XSMT isn't high, there's no sound.
- **PCM1808 pins:** FMT = GND (I2S), MD0 = MD1 = GND (slave mode). Input on VINL; leave VINR unconnected.
- PCM1808 full scale ≈ 0.6 × VCC ≈ 3 Vpp at 5 V. A guitar peaks around 1–2 Vpp, so there's headroom.

#### Input buffer
```
J1 tip ─┬── C1 100nF film ──┬────────── U4 pin 3 (+IN A)
        │                   │           U4 pin 2 ── pin 1   (follower)
       R1 1M               R2 1M        U4 pin 1 ── C3 10µF (+ toward TL072) ── U2 VINL
        │                   │
       GND                Vref ── C2 47µF ── GND
                            │
                9V ─ R3 100k ┴ R4 100k ─ GND

U4 pin 8 = 9 V (+ C4 100nF to GND), pin 4 = GND
Unused half: pin 5 → Vref, pin 6 ↔ pin 7
```

#### Output
```
U3 LOUT ── R5 1k ── RV8 (A10K) top;  RV8 wiper ── J2 tip;  RV8 bottom ── GND
```
The PCM5102A output is ground-centered: no coupling capacitor needed.

#### Grounding
- One **star ground point**: 9 V return, regulator, ESP32, converters, buffer.
- Keep the analog input wires (J1 → buffer → U2) **short and away** from the ESP32 and the clock wires.
- **Wi-Fi and Bluetooth off** (also required for RV7 on GPIO 4).

---

## 9. Firmware

A PlatformIO (Arduino-ESP32) project in `firmware/` that includes `../pedal.h` **unchanged**.

### 9a. Structure
| Part | What it does |
|---|---|
| **Audio task** (core 1) | I2S at 48 kHz, 32-bit slots (24-bit data), master with MCLK. Loop: read a block of 32–64 samples → float ±1.0 → `Pedal::process()` per sample → back to int → write |
| **Control task** (core 0) | Every ~10 ms: read the 7 pots (average 8–16 readings) → 0–10 → `set("tsDriveKnob", v)` etc. **only when a knob moved more than ~0.05** (hysteresis, so pot noise doesn't keep recomputing). Read the pull switch → `kaDirty`. Debounce the footswitches → toggle `tsOn` / `klonOn` / `katanaOn` on press, drive the LEDs |
| **Settings handoff** | Settings change between audio blocks (a flag or a short critical section), never in the middle of one |
| **Baked config** | A tuned preset from the tuner ("Copy config"), compiled in as a string and applied at boot with `Pedal::apply()`. It holds the circuit internals; the pots override the front-panel values |
| **Serial config** (stage 2) | Lines typed or pasted over USB serial go straight to `Pedal::apply()`, the same format as `presets/*.cfg`. Tune the circuit internals live on the real hardware |

### 9b. Steps
1. [ ] Create `firmware/` (PlatformIO, `esp32dev`, Arduino framework), with an include path to the repo root for `pedal.h`
2. [ ] I2S passthrough (no effects): check latency (< 3 ms) and noise
3. [ ] Add `Pedal`, all effects on, fixed settings
4. [ ] Pots + footswitches + LEDs
5. [ ] Baked config from the tuner
6. [ ] Serial config
7. [ ] **2× oversampling** around the clipping stages if high-drive fizz (aliasing) is audible
8. [ ] CPU check: time `process()` per block. Expect a few % of one core

### 9c. Getting settings from the tuner to the pedal
| Stage | How |
|---|---|
| 1. Bake in | Tuner → **Copy config** → paste into `firmware/src/config.h` → flash |
| 2. Live over USB | Serial monitor → paste config lines → applied instantly (later: a "Send to pedal" button in the tuner) |
| 3. Presets in flash | Several `.cfg` presets stored in the ESP32's flash, switched by a 4th footswitch. Physical pots disagree with a loaded preset, so use **pickup mode** (a knob takes over once moved) or encoders + OLED |

---

## 10. Compare analog vs digital

### In software (now)
The tuner already compares the model with the **real TS9 recording**: press **2 / 3**, read the tone-difference score, and check the spectrum. See [2b](#2b-tuning-workflow).

### On hardware (after phases 2 and 4)

**Expected differences at first:**
| | Analog TS | Digital model |
|---|---|---|
| Filters | Several RC filters that interact | One-pole approximations |
| Clipping | Real 1N914s + the 4558's limits | `sat()` math curve |
| Pickup loading | The pedal's input loads the pickups | Hidden behind the 1 MΩ buffer |
| Aliasing (harsh fizz) | None | Possible at high drive |
| Noise | Op-amp hiss | Converter noise + possible ESP32 hum |
| Consistency | Part tolerances | Identical every time |

**How to compare fairly:**
- [ ] **Match the volume**: multimeter on AC volts with a constant note, or by ear. The louder pedal always sounds "better"
- [ ] **Reamp**: record one dry track, play it from the PC → passive DI (or a 10:1 divider, 10 kΩ / 1 kΩ) → pedal → record. Same input for both. Start with the volume low
- [ ] **Record your analog TS** with the same dry track and load the pair in the tuner as `NAME_in.wav` + `NAME_out.wav`: it becomes the "Real pedal" reference
- [ ] **Blind A/B**: a friend switches without telling you which is which
- [ ] Repeat for the Klon / Katana if built

**Possible end result:** a hybrid board, with the real TS808 followed by the ESP32 doing Klon + Katana.

---

## 11. Permanent build

- [ ] Only start once the breadboard version sounds right
- [ ] Draw the schematic in **KiCad** (input buffer, power, connectors, module headers, 7 pots, switches)
- [ ] **Stripboard** first, **or** a PCB from JLCPCB / PCBWay (~€5 for 5 boards + shipping)
- [ ] Separate analog and digital ground areas, joined at one point
- [ ] **1590XX / 125B**: mark → center punch → step drill, following the [front-panel layout](#enclosure-buy-after-the-breadboard-version-works)
- [ ] Mount the converter modules on headers (swappable)

---

## 12. Risks, gotchas, later ideas

### Gotchas
| Symptom | Likely cause |
|---|---|
| No sound from the DAC | XSMT not high; SCK not grounded; wrong I2S format |
| No sound from the ADC | Missing MCLK; MD0/MD1 not grounded; DIN not on GPIO 19 in the firmware |
| Can't flash the ESP32 | GPIO0 (MCLK) is the boot pin: unplug MCLK while flashing |
| Katana VOLUME (GPIO 4) reads garbage | ADC2 conflicts with Wi-Fi: make sure Wi-Fi is off |
| Knob jumps near the ends of travel | The ESP32's ADC is nonlinear near 0 V and 3.3 V: map the usable range (≈0.1–3.1 V) to 0–10 |
| Settings flicker / zipper noise | Pot noise: more averaging, hysteresis, the 100 nF wiper caps |
| Dull tone | Input impedance too low (R2 = 1 MΩ? buffer working?) |
| Hum / buzz | Ground loop, long analog wires, Wi-Fi on, non-isolated supply |
| Whine that follows ESP32 activity | Digital noise on the power rail: decoupling, separate analog supply |
| Fizz at high drive | Aliasing: oversample |
| Analog pedal: no sound / no LED | Diode or electrolytic in backwards; check Vref = 4.5 V |
| Klon: weak / distorts early | Charge pump not giving ~18 V |

### Unverified
- The TONE / LEVEL / OUTPUT / VOLUME knob mappings and the Katana dirty mode are approximations. DRIVE uses the real TS808 gain formula.
- The analog part values in section 5 come from the TS808 schematic from memory: **your kit's BOM wins**.
- No verified Katana schematic was found.
- Shop stock and prices weren't checked.
- The firmware isn't written yet; ESP32 timing and noise are estimates.

### Later ideas
- Delay / reverb (WROVER PSRAM)
- OLED + encoder menu, presets in flash, pickup mode for the pots
- "Send to pedal" button in the tuner (serial)
- Headphone output with the amp + cab sim running on the ESP32
- Expression pedal, tap tempo
- More accurate models in `pedal.h` (biquads, diode equation), fitted to recordings of your own analog builds
