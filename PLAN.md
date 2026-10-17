# Magic Mayer Home-Made Pedal: Plan

## Goal

Build three guitar effects and switch each one on or off on its own:

- **Tube Screamer (TS808/TS9):** overdrive with a mid boost.
- **Klon Centaur:** overdrive that mixes clean and distorted signal.
- **Keeley Katana:** clean boost, with an optional dirty mode.

There are two versions:

- **Branch A, analog:** real circuits (op-amps, diodes, resistors, capacitors). One box per effect. Only the Tube Screamer is required; it is the reference.
- **Branch B, digital:** one ESP32 box. The guitar signal is converted to numbers, `pedal.h` processes it, and it is converted back to audio.

The PC software is already done. The `tuner` app and the `sim` tool run the same `pedal.h` the ESP32 will run, so the sound can be tuned before buying any parts.

What you learn: analog circuits (buffers, filters, clipping) and DSP (the same filters written as code).

---

## Contents

1. [Overview and timeline](#1-overview-and-timeline)
2. [Phase 0: software](#2-phase-0-software)
3. [Controls](#3-controls)
4. [Parts for both branches](#4-parts-for-both-branches)
5. [Branch A: analog](#5-branch-a-analog)
6. [Branch B: digital (ESP32)](#6-branch-b-digital-esp32)
7. [Shopping list by shop](#7-shopping-list-by-shop)
8. [Build steps](#8-build-steps)
9. [Firmware](#9-firmware)
10. [Comparing analog and digital](#10-comparing-analog-and-digital)
11. [Final build](#11-final-build)
12. [Problems and fixes](#12-problems-and-fixes)

---

## 1. Overview and timeline

```
Branch A (analog):   Guitar -> [TS808] -> [Klon]* -> [Katana]* -> Amp       * optional
                              one true-bypass footswitch per box

Branch B (digital):  Guitar -> TL072 buffer -> PCM1808 -> ESP32 (pedal.h) -> PCM5102A -> volume pot -> Amp
                              1 MOhm input   ADC        3 effects          DAC
                                                        7 pots, 1 pull switch, 3 footswitches

PC (done):           recording -> tuner / sim (same pedal.h) -> presets/*.cfg -> copied into the firmware

Possible final setup: Guitar -> [analog TS808] -> [ESP32 doing Klon + Katana] -> Amp
```

| Phase | Task | Branch | Time | Status |
|---|---|---|---|---|
| 0 | Software: tuner, simulator, real TS9 recording | - | - | Done |
| 0b | Tune presets in the tuner against the real TS9 | - | a few evenings | To do |
| 1 | Order parts | both | 1 evening + 1-3 weeks shipping | To do |
| 2 | Build the analog TS kit | A | 3-4 h | To do |
| 2b | Build the analog Klon / Katana (optional) | A | Klon 4-6 h, Katana 1-2 h | To do |
| 3 | Build the digital pedal on a breadboard | B | 6-10 h | To do |
| 4 | Write the firmware | B | 2-3 evenings | To do |
| 5 | Compare and tune on real hardware | both | open-ended | To do |
| 6 | Final digital build in an enclosure | B | 1-2 weekends | To do |

Order: tune in software now. Order parts. Parts from Musikding arrive in about 1 week, AliExpress in 2-3 weeks. Build the analog TS while waiting for the AliExpress parts.

---

## 2. Phase 0: software

### 2a. What exists

| File | Purpose |
|---|---|
| `pedal.h` | The three effects. Used by the tuner, by `sim` and later by the ESP32. Every setting has a name and can be loaded from text |
| `tuner` | Desktop app. Plays audio through `pedal.h` in real time. Has the real front-panel knobs, extra circuit settings, instant switching between dry / model / real pedal while the audio plays (same loudness, so you hear only the tone difference), an amp + speaker simulation for listening, plots and presets |
| `sim` | Command-line tool. Renders every effect combination to `renders/` and runs self-checks |
| `recordings/` | A real Telecaster recorded dry, and the same signal through a real Ibanez TS9 (drive max, tone and level at noon). From GuitarML |
| `presets/*.cfg` | Saved settings: `default`, and `ts9_max_drive` with the same knobs as the recording |

```bash
make deps && make recordings && make
./tuner                                   # or drag a dry guitar WAV onto the window
./sim [guitar.wav] [presets/NAME.cfg]
```

### 2b. How to tune

- [ ] Load the `ts9_max_drive` preset. Press 2 and 3 to switch between the model and the real pedal. Change the TS circuit settings (mid hump, tone corner, gain) until the "tone difference" number goes down.
- [ ] Match the compression of the real TS9: its output level changes about 6 dB while the input changes about 15 dB.
- [ ] Check the harmonics plot. A symmetric clipper like the TS makes mostly odd harmonics (3rd, 5th).
- [ ] Record your own guitar dry (any audio interface, 16 or 24-bit WAV) and drag it in. Different pickups change the result a lot.
- [ ] Save one preset per sound, for example `ts-lead`, `klon-push`, `katana-clean`.
- [ ] Turn "Amp + cab" off sometimes. The ESP32 outputs only the pedal, so the sound must work without the amp simulation.

### 2c. Analog circuit simulation (free, to learn the TS circuit)

| Tool | Use |
|---|---|
| Falstad (falstad.com/circuit, in the browser) | Quick and visual. Shows the signal and the clipping live |
| KiCad + ngspice (`sudo dnf install kicad`) | Frequency-response plots. The same tool can later make a PCB |
| LTspice (Windows/Mac, or Wine on Linux) | Many ready-made Tube Screamer files exist online |

- [ ] Draw the TS808 clipping stage: op-amp, 4.7 kOhm + 47 nF to Vref, 51 kOhm + 500 kOhm drive pot and 51 pF in the feedback loop, two 1N914 diodes in the feedback loop.
- [ ] Find the high-pass corner: f = 1 / (2 * pi * 4.7 kOhm * 47 nF) = about 720 Hz. In code this is `tsHpHz`.
- [ ] Check the gain formula of the DRIVE knob: 1 + (51 kOhm + R_drive) / 4.7 kOhm = 12x to 118x. In code this is `applyPanel()`.

### 2d. Testing the controls without hardware (optional)

- [ ] Wokwi (wokwi.com) simulates an ESP32 with pots, buttons and LEDs in the browser. It cannot simulate the I2S audio.

---

## 3. Controls

The tuner and the pedal both use the real front panels with a 0-10 scale. `Pedal::applyPanel()` converts knob positions into circuit values. The sliders in the tuner and the pots on the pedal use the same code.

| Pedal | Real control | Config key | What it changes in the model |
|---|---|---|---|
| TS808 / TS9 | DRIVE | `tsDriveKnob` | `tsDrive` = 1 + (51k + 500k log pot) / 4.7k, so 11.9x to 118x. This is the real TS808 formula |
| | TONE | `tsToneKnob` | `tsToneHz` from 500 Hz to 6 kHz (approximation) |
| | LEVEL | `tsLevelKnob` | `tsLevel`: off, then -24 to +6 dB |
| Klon Centaur | GAIN | `klGainKnob` | `klGain` 1x to 60x and `klBlend` clean to dirty. One knob moves both, like the real Klon's dual pot |
| | TREBLE | `klTrebleKnob` | `klTreble` 1/3 to 3, flat at the middle |
| | OUTPUT | `klOutputKnob` | `klLevel`: off, then -24 to +12 dB |
| Keeley Katana | VOLUME | `kaVolumeKnob` | `kaBoost`: off, then -20 to +35 dB |
| | push/pull | `kaDirty` | Dirty mode: more gain into soft clipping (approximation) |
| all | footswitch | `tsOn`, `klonOn`, `katanaOn` | Effect on or off |

**Circuit settings** are values inside the pedal that have no knob: `tsHpHz`, `klClip`, `klBpHz`, `klTrebleHz`, `kaBright`, `kaShelfHz`. They can also override what the knobs set. Tune them in the tuner, then copy them into the firmware ([section 9](#9-firmware)).

In config files, knobs come first and circuit values after. Setting a knob recalculates its circuit values. Lines after it can override them.

---

## 4. Parts for both branches

### Power

| Qty | Part | Notes |
|---|---|---|
| 1 | 9 V DC pedal power supply, center-negative, 2.1 mm plug, at least 500 mA. Isolated outputs are better | Isolated outputs avoid hum when several pedals share one supply. A simple adapter with a daisy-chain cable is fine to start |
| 1 | Daisy-chain cable (2.1 mm, 3-5 plugs) | Only needed with a non-isolated supply |

### Cables

| Qty | Part |
|---|---|
| 3-4 | Short patch cables, 15-30 cm, 6.35 mm mono |
| 1 | Guitar cable |

### Tools

| Qty | Part | Notes |
|---|---|---|
| 1 | Multimeter | Required. DC volts (bias), continuity (wiring), AC volts (level matching) |
| 1 | Soldering iron with temperature control (Pinecil, TS101 or any 60 W station) | |
| 1 | Solder 0.6-0.8 mm. Leaded 60/40 or 63/37 is easier; lead-free also works | |
| 1 | Desoldering pump and desoldering wick | |
| 1 | Flush side cutters and wire strippers | |
| 1 | PCB holder ("helping hands") | |
| 1 | Solid-core wire, 22-24 AWG, several colors | |
| 1 | Heat-shrink tubing set | |
| 1 | Step drill bit and center punch | For drilling the enclosures |
| 1 | Headphones or a small powered speaker | For testing without an amp |
| 1 | USB audio interface (if you don't have one) | To record your own dry guitar and to reamp |

---

## 5. Branch A: analog

Each pedal is its own box: PCB, parts, enclosure, 3PDT true-bypass footswitch, jacks, DC jack and LED.

Buy **complete kits** (PCB, all parts, drilled enclosure). A kit comes with its exact parts list (BOM), and that list is always correct. The tables below explain what each part does, so you can follow the circuit. The values come from the TS808 schematic (ElectroSmash analysis) from memory. Check them against your kit's BOM.

### A0. Hardware every analog pedal needs

| Qty per pedal | Part | Notes |
|---|---|---|
| 1 | 3PDT latching footswitch | True bypass. Latching = it clicks on and stays on. Branch B uses momentary switches instead |
| 2 | 6.35 mm mono jacks (open-frame) | Input and output |
| 1 | 2.1 mm DC jack, isolated | Center-negative |
| 1 | LED (3 or 5 mm) and bezel | On/off light |
| 1 | LED resistor, 2.2-4.7 kOhm | Higher value = dimmer |
| 1 | 3PDT LED board (optional) | Makes the footswitch wiring easier |
| 1 | Enclosure: 1590B (TS, Katana) or 125B / 1590BB (Klon) | Pre-drilled if the shop offers it |
| - | Knobs, one per pot | |
| 1 | 9 V battery clip (optional) | |

### A1. Tube Screamer TS808 (required, the reference)

Buy a TS808 kit from Musikding.de, or any TS808 PCB with parts.

| Stage | Parts | What it does |
|---|---|---|
| Power | 1N4001 (polarity protection), 2x 10 kOhm divider + 47 uF = Vref 4.5 V, 100 uF filter cap | Makes a 4.5 V middle point so the op-amp can swing up and down on a single 9 V supply |
| Bypass (original only) | JFETs, transistors, flip-flop | Kits replace this with the 3PDT switch. Skip it |
| Input buffer | 2SC1815 transistor (emitter follower), ~20 nF input cap, 510 kOhm bias resistors, 10 kOhm emitter resistor | High input impedance, so the guitar pickups are not loaded |
| Clipping stage | JRC4558D op-amp (first half). 4.7 kOhm + 47 nF to Vref = 720 Hz high-pass, which creates the mid hump. 51 kOhm + 500 kOhm (log) DRIVE pot in the feedback loop. 51 pF across the loop cuts high frequencies. 2x 1N914/1N4148 diodes back to back in the loop | Gain 12x to 118x. The diodes in the feedback loop give the soft TS clipping |
| Tone and level | 1 kOhm + 220 nF fixed low-pass (~723 Hz), 20 kOhm TONE pot, op-amp second half as active tone stage, 220 Ohm, 1 kOhm, 220 nF, 100 kOhm (log) LEVEL pot | Treble cut/boost and output volume |
| Output buffer | 2SC1815 (emitter follower), 100 Ohm series resistor (TS9: 470 Ohm), 10 kOhm pull-down, 1 uF / 10 uF coupling caps | Low output impedance to drive the cable |

Pots: DRIVE 500 kOhm log (A), TONE 20 kOhm (W/G taper, or B), LEVEL 100 kOhm log (A).
About 25 resistors, 15 capacitors, 1 op-amp with a DIP-8 socket (so you can try other op-amps: JRC4558, TL072, RC4558), 2-3 transistors, 3 diodes, 3 pots.

Every filter in the TS part of `pedal.h` matches a part on this board.

### A2. Klon Centaur clone (optional)

Buy a complete Klon clone kit: PedalPCB "Kliche", Aion FX "Refractor", the General Guitar Gadgets replica kit, or the one from Musikding.

| Part | What it does |
|---|---|
| Charge pump (TC1044SCPA / ICL7660 / LT1054) | Doubles 9 V to about 18 V. More headroom = louder before it distorts |
| 2x TL072 op-amps with sockets | Gain stage, mixing stage, tone, output |
| 2x 1N34A germanium diodes | Hard clipping to ground. In code: `klClip` |
| Dual 100 kOhm (B) GAIN pot | One half sets the gain, the other half mixes clean and dirty. In code: `klGainKnob` sets `klGain` and `klBlend` |
| 10 kOhm (B) TREBLE pot | Active treble control (`klTrebleKnob`) |
| 10 kOhm (B) OUTPUT pot | Volume (`klOutputKnob`) |
| About 60 small parts | 4-6 h of soldering. The hardest board in this project |

Enclosure: 125B.

### A3. Keeley Katana clone (optional)

Buy a Katana-style boost PCB. The PedalPCB forum ("Cleaver" boost) and Aion FX have layouts. Check which Katana version (v1 or v2) the layout copies.
Main parts: JFETs, a charge pump for 18 V headroom, and a VOLUME pot (push/pull for the dirty mode on versions that have it).
Small board, 1-2 h, 1590B enclosure.

No verified Katana schematic was found. Use the BOM that comes with the PCB.

### Branch A cost (approximate)

| Build | Cost |
|---|---|
| TS808 kit, enclosure, hardware | 35-60 EUR |
| Klon kit | 40-80 EUR |
| Katana PCB and parts | 25-40 EUR |

---

## 6. Branch B: digital (ESP32)

One box: input buffer, converters, ESP32, the real front-panel knobs, and three momentary footswitches.

### B1. Parts list (BOM)

#### ESP32 and audio converters

| Ref | Qty | Part | Notes |
|---|---|---|---|
| U1 | 1 (+1 spare) | ESP32 DevKit (ESP32-WROOM-32, 38 pins) | Must have every GPIO from the [pin plan](#b2-pin-plan) |
| U2 | 1 (+1-2 spares) | PCM1808 module (ADC: audio in) | Needs a master clock (MCLK) from the ESP32 |
| U3 | 1 (+1-2 spares) | PCM5102A module (GY-PCM5102, DAC: audio out) | Set the jumpers on the back |
| - | 1 (alternative) | Digilent Pmod I2S2 | Replaces U2 and U3 with one board (DigiKey/Mouser) |

#### Input buffer

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| U4 | 1 (+4 spares) | TL072 op-amp (DIP-8) with socket | - | Buffer |
| R1 | 1 | resistor | 1 MOhm | Pull-down on the input jack, prevents pops |
| C1 | 1 | film capacitor | 100 nF | Blocks DC from the guitar |
| R2 | 1 | resistor | 1 MOhm | Biases the op-amp to Vref. Also sets the 1 MOhm input impedance the pickups need |
| R3, R4 | 2 | resistor | 100 kOhm | Divider, 9 V to 4.5 V (Vref) |
| C2 | 1 | electrolytic | 47 uF, 16 V or more | Keeps Vref stable |
| C3 | 1 | electrolytic | 10 uF, 16 V or more | Coupling cap to the PCM1808 input |
| C4 | 1 | ceramic | 100 nF | Supply decoupling, placed at TL072 pin 8 |
| - | - | - | - | Unused TL072 half: pin 5 to Vref, pin 6 to pin 7. This stops it from oscillating |

#### Output

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| RV8 | 1 | pot | 10 kOhm log (A) | Master volume. The PCM5102A outputs about 2.1 Vrms, which is too loud for an amp input. Analog only, the ESP32 does not read it |
| R5 | 1 | resistor | 1 kOhm | Protects the output if it is shorted |

#### Power

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| J3 | 1 | DC jack 2.1 mm | center-negative | 9 V input |
| D1 | 1 | Schottky diode | 1N5817 | Protects against a reversed power supply |
| C5 | 1 | electrolytic | 100 uF, 16 V or more | Smooths the 9 V |
| U5 | 1 (+1-2 spares) | 5 V regulator module | AMS1117-5.0 or 7805 | 9 V to 5 V for the ESP32 and converters |
| C6, C7 | 2 | electrolytic | 10 uF | Regulator input and output (if the module has none) |
| C8-C10 | 3 | ceramic | 100 nF | Decoupling next to U1, U2, U3 |

The TL072 runs directly from 9 V (it needs at least 7 V). The ESP32 makes its own 3.3 V from the 5 V.

#### Front panel (the real pedals' controls)

| Ref | Qty | Part | Value | Function |
|---|---|---|---|---|
| RV1 | 1 | pot | 10 kOhm linear (B) | TS DRIVE |
| RV2 | 1 | pot | 10 kOhm B | TS TONE |
| RV3 | 1 | pot | 10 kOhm B | TS LEVEL |
| RV4 | 1 | pot | 10 kOhm B | Klon GAIN (one pot is enough, the code moves gain and mix together) |
| RV5 | 1 | pot | 10 kOhm B | Klon TREBLE |
| RV6 | 1 | pot | 10 kOhm B | Klon OUTPUT |
| RV7 | 1 | push/pull pot (pot with a switch) | 10 kOhm B | Katana VOLUME. Pulling it turns on dirty mode |
| C11-C17 | 7 | ceramic | 100 nF | From each pot's middle pin to GND. Reduces noise in the readings |
| SW1-SW3 | 3 | momentary switch | buttons on the breadboard, soft-touch footswitches in the final build | TS / Klon / Katana on-off. Momentary, not latching: the ESP32 remembers the state |
| LED1-LED3 | 3 | LED 3/5 mm | - | Shows which effects are on |
| R6-R8 | 3 | resistor | 1 kOhm (brighter) to 4.7 kOhm (dimmer) | LED current limit |

All pots are linear (B), even where the real pedal uses a log pot. The code applies the log curve (`Pedal::logTaper`), so the hardware stays simple.

#### Connectors and prototyping

| Ref | Qty | Part |
|---|---|---|
| J1, J2 | 2 | 6.35 mm mono jacks (in, out) |
| - | 2 | Breadboards, 830 points |
| - | 1 pack each | Jumper wires male-male and male-female |
| - | 1 | Pin headers (to solder onto the modules) |
| - | 2 | Stripboard or perfboard |
| - | 1 each | Resistor kit and capacitor kit |

#### Enclosure (buy after the breadboard version works)

| Qty | Part |
|---|---|
| 1 | 1590XX or 125B, used in landscape. More room than a 1590BB for 7 knobs and 3 footswitches |
| 7 | Knobs (one that is easy to pull for RV7) |
| 3 | LED bezels |

```
 +----------------------------------------------+
 | (DRV)(TONE)(LVL)  (GAIN)(TRB)(OUT)  (VOL)    |   VOL = push/pull
 |      o TS             o KLON         o KAT   |   o = LED
 |     [ FS ]           [ FS ]         [ FS ]   |   FS = footswitch
 |                                   (MASTER)   |   master volume, jacks and DC on the side or top
 +----------------------------------------------+
```

#### Extra parts for later (optional, cheap now)

| Qty | Part | Allows |
|---|---|---|
| 1 | ESP32-WROVER (4-8 MB PSRAM) | Delay and reverb (they need a lot of memory) |
| 1-2 | SSD1306 0.96" OLED display (I2C) | Show settings and preset names |
| 2 | EC11 rotary encoder with push button | Change circuit settings and presets on the pedal |
| 1-2 | Extra soft-touch footswitches | Preset switching, tap tempo |
| 1 | 6.35 mm stereo (TRS) jack | Expression pedal or headphone output |

### B2. Pin plan

| Function | ESP32 GPIO | Notes |
|---|---|---|
| I2S MCLK | 0 | 12.288 MHz (256 x 48 kHz). This is also the boot pin: disconnect it if flashing fails |
| I2S BCK | 26 | Shared by both converters |
| I2S WS / LRCK | 25 | Shared |
| I2S data out to PCM5102A DIN | 22 | |
| I2S data in from PCM1808 OUT | 19 | Moved from 33 to free an analog pin |
| RV1 TS DRIVE | 32 | ADC1 |
| RV2 TS TONE | 33 | ADC1 |
| RV3 TS LEVEL | 34 | ADC1, input only |
| RV4 Klon GAIN | 35 | ADC1, input only |
| RV5 Klon TREBLE | 36 (VP) | ADC1, input only |
| RV6 Klon OUTPUT | 39 (VN) | ADC1, input only |
| RV7 Katana VOLUME | 4 | ADC2. Only works while Wi-Fi is off. The firmware keeps Wi-Fi off |
| RV7 pull switch (dirty mode) | 23 | To GND, internal pull-up |
| SW1 / SW2 / SW3 | 13 / 14 / 27 | To GND, internal pull-ups, 20-30 ms debounce |
| LED1 / LED2 / LED3 | 16 / 17 / 18 | Through R6-R8 |
| Reserved: OLED I2C SDA / SCL | 21 / 5 | For later |

Pot wiring: one outer pin to 3.3 V (never 5 V), the other outer pin to GND, the middle pin to the GPIO, with a 100 nF cap (C11-C17) from the middle pin to GND.

### Branch B cost (approximate)

| Part | Cost |
|---|---|
| Converter modules (with spares) | 10-20 EUR |
| Small parts, TL072, regulator, breadboards, wires | 20-35 EUR |
| Front panel: 6 pots, 1 push/pull pot, buttons, LEDs | 15-25 EUR |
| Jacks, DC jack, footswitches (final) | 15-25 EUR |
| Enclosure and knobs (final) | 20-30 EUR |
| Extra parts (optional) | 15-25 EUR |

---

## 7. Shopping list by shop

Columns A and B: "yes" = needed for that branch, "opt" = optional, "-" = not needed.

### Musikding.de (pedal parts, about 1 week to Denmark). Order first

| Item | Qty | A | B |
|---|---|---|---|
| TS808 kit (complete, with enclosure if offered) | 1 | yes | - |
| Klon kit | 1 | opt | - |
| 3PDT latching footswitches (if not in the kits) | 1 per pedal | yes | - |
| Soft-touch momentary footswitches | 3 (+2) | - | yes |
| 10 kOhm (B) push/pull pot | 1 (+1) | - | yes |
| 6.35 mm mono jacks | 2 per pedal + 2 | yes | yes |
| 6.35 mm stereo jack | 1 | - | opt |
| 2.1 mm DC jacks | 1 per box | yes | yes |
| 1590XX or 125B enclosure | 1 (later) | - | yes |
| Knobs, LED bezels | as needed | yes | yes |
| Spare TS pots: 500 kOhm A, 20 kOhm W/B, 100 kOhm A | 1 each | opt | - |
| JRC4558D op-amps (to try swapping) | 2 | opt | - |
| 1N914, 1N34A, LEDs as clipping diodes (to experiment) | a few | opt | - |

### AliExpress / eBay.de (cheap, 1-3 weeks). Order on day 1

| Item | Qty | A | B |
|---|---|---|---|
| PCM1808 module | 2-3 | - | yes |
| PCM5102A module | 2-3 | - | yes |
| ESP32 DevKit (spare) | 1 | - | yes |
| ESP32-WROVER | 1 | - | opt |
| SSD1306 OLED | 1-2 | - | opt |
| EC11 encoders | 2 | - | opt |

Faster options in the EU: PCM5102A from Electrokit (Sweden) or Otronic (Netherlands). Pmod I2S2 from DigiKey/Mouser.

### Danish hobby shop (Let Elektronik / Elektronik-Lavpris / Ardustore / Elextra)

| Item | Qty | A | B |
|---|---|---|---|
| TL072 (DIP-8) and DIP-8 sockets | 5 + 5 | opt | yes |
| AMS1117-5.0 or 7805 regulator module | 2-3 | - | yes |
| 1N5817 diodes | 5 | - | yes |
| Resistor kit (E12, 1/4 W, with 1 MOhm, 100 kOhm, 1 kOhm, 4.7 kOhm) | 1 | yes | yes |
| Capacitor kit: ceramic 100 nF (at least 20), electrolytic 10/47/100 uF | 1 each | yes | yes |
| Film capacitors 100 nF | 5 | - | yes |
| Breadboards, 830 points | 2 | opt | yes |
| Jumper wires (M-M, M-F), pin headers | 1 pack each | - | yes |
| Tactile buttons | 5 | - | yes |
| LEDs (3/5 mm) | 10 | yes | yes |
| B10K linear pots | 6 (+2 spare) | - | yes |
| A10K log pot (master volume) | 1-2 | - | yes |
| Stripboard / perfboard | 2 | - | yes |
| Wire, heat shrink | 1 each | yes | yes |

### Thomann (if you don't have them)

| Item | Qty | A | B |
|---|---|---|---|
| 9 V isolated pedal power supply | 1 | yes | yes |
| Patch cables | 3-4 | yes | yes |
| USB audio interface (2 inputs) | 1 | opt | opt |

### DigiKey / Mouser / Farnell (if something is out of stock)

Free shipping above a minimum order. Good for: TL072, TC1044SCPA, JRC4558D, film caps, Pmod I2S2, push/pull pots.

---

## 8. Build steps

### 8A. Analog TS (1 evening)

1. [ ] Sort the parts. Measure every resistor with the multimeter.
2. [ ] Solder the lowest parts first: resistors, diodes (check the stripe direction), IC socket, small caps, transistors, electrolytic caps (check polarity), pots.
3. [ ] Wire the jacks, DC jack, 3PDT switch and LED.
4. [ ] Before inserting the op-amp: power on and measure 9 V at socket pin 8 and 4.5 V at Vref.
5. [ ] Insert the 4558 (pin 1 marked with a dot) and play.
6. [ ] Find each stage from the [A1 table](#a1-tube-screamer-ts808-required-the-reference) on the board.
7. [ ] Experiments: swap op-amps, replace the 1N914 diodes with 1N34A or LEDs, change C3 from 47 nF to 100 nF. Try the same changes in the tuner's circuit settings.

### 8B. Analog Klon / Katana (optional)

Same order. Klon: measure about 17-18 V at the charge pump output before inserting the TL072s.

### 8C. Digital pedal on a breadboard (1 weekend)

Build in this order. Test each step before the next one.

1. [ ] Power: 9 V to D1, C5, U5. Measure 5.0 V before connecting anything else.
2. [ ] ESP32 alone: flash a blink program. Never power it from USB and the 5 V rail at the same time.
3. [ ] Output: connect U3 with the jumpers set. Play a 440 Hz sine wave and listen through RV8 with headphones.
4. [ ] Input: connect U2 with MCLK. Pass the input straight to the output. Test with phone audio first.
5. [ ] Buffer: build the TL072 stage, then connect the guitar. The sound must not be dull, hum or distort on hard strums.
6. [ ] Front panel: RV1-RV7 and the pull switch. Print the 0-10 values over serial and turn every knob from end to end.
7. [ ] Footswitches and LEDs: SW1-SW3, LED1-LED3.
8. [ ] Firmware with `pedal.h`: see [section 9](#9-firmware).

#### I2S wiring (the ESP32 is the clock master)

| ESP32 | PCM1808 (U2) | PCM5102A (U3) |
|---|---|---|
| GPIO0 (MCLK) | SCK | - (connect the board's SCK to GND: it then uses its internal PLL) |
| GPIO26 (BCK) | BCK | BCK |
| GPIO25 (WS / LRCK) | LRC | LCK |
| GPIO19 (data in) | OUT | - |
| GPIO22 (data out) | - | DIN |
| 5 V / GND | VCC (5 V), VDD (3.3 V) / GND | VIN / GND |

- PCM5102A jumpers on the back: FLT = L, DEMP = L, XSMT = H, FMT = L. If XSMT is not high, there is no sound.
- PCM1808 pins: FMT = GND (I2S format), MD0 = MD1 = GND (slave mode). Use the VINL input, leave VINR unconnected.
- PCM1808 maximum input is about 0.6 x VCC = 3 Vpp at 5 V. A guitar peaks at about 1-2 Vpp, so it fits.

#### Input buffer

```
J1 tip --+-- C1 100nF film --+----------- U4 pin 3 (+IN A)
         |                   |            U4 pin 2 -- pin 1   (voltage follower)
        R1 1M               R2 1M         U4 pin 1 -- C3 10uF (+ side to TL072) -- U2 VINL
         |                   |
        GND                Vref -- C2 47uF -- GND
                             |
                 9V -- R3 100k --+-- R4 100k -- GND   (the middle point is Vref)

U4 pin 8 = 9 V (C4 100nF to GND), pin 4 = GND
Unused half: pin 5 to Vref, pin 6 to pin 7
```

#### Output

```
U3 LOUT -- R5 1k -- RV8 (A10K) top pin;  RV8 middle pin -- J2 tip;  RV8 bottom pin -- GND
```

The PCM5102A output has no DC on it, so no coupling capacitor is needed.

#### Grounding

- Use one ground point ("star ground") for the 9 V return, regulator, ESP32, converters and buffer.
- Keep the input wires (J1 to buffer to U2) short and away from the ESP32 and the clock wires.
- Wi-Fi and Bluetooth off. This is also required for RV7 on GPIO 4.

---

## 9. Firmware

A PlatformIO project (Arduino framework for ESP32) in `firmware/`. It includes `../pedal.h` without changes.

### 9a. Structure

| Part | What it does |
|---|---|
| Audio task (core 1) | I2S at 48 kHz, 32-bit slots with 24-bit data, ESP32 as master with MCLK. Loop: read a block of 32-64 samples, convert to float (-1 to +1), run `Pedal::process()` on each sample, convert back, write the block |
| Control task (core 0) | Every 10 ms: read the 7 pots (average 8-16 readings each), scale to 0-10, call `set("tsDriveKnob", v)` and so on, but only when a knob moved more than about 0.05 (hysteresis: pot noise does not trigger recalculation). Read the pull switch into `kaDirty`. Debounce the footswitches, toggle `tsOn` / `klonOn` / `katanaOn` on each press, update the LEDs |
| Settings handoff | Settings change between audio blocks (a flag or a short critical section), never in the middle of a block |
| Built-in config | A preset from the tuner ("Copy config") is pasted into the code as text and applied at startup with `Pedal::apply()`. It holds the circuit settings. The pots then set the knob values |
| Serial config (stage 2) | Lines sent over USB serial go to `Pedal::apply()`. Same format as `presets/*.cfg`. Allows tuning circuit settings live on the real pedal |

### 9b. Steps

1. [ ] Create `firmware/` (PlatformIO, board `esp32dev`, Arduino framework) with the repository root in the include path, for `pedal.h`.
2. [ ] I2S input passed straight to the output, no effects. Check latency (under 3 ms) and noise.
3. [ ] Add `Pedal` with all effects on and fixed settings.
4. [ ] Add pots, footswitches and LEDs.
5. [ ] Add the built-in config from the tuner.
6. [ ] Add serial config.
7. [ ] Add 2x oversampling around the clipping stages if harsh high-frequency fizz (aliasing) is audible at high drive.
8. [ ] Measure CPU use: time `process()` per block. Expected: a few percent of one core.

### 9c. Moving settings from the tuner to the pedal

| Stage | How |
|---|---|
| 1. Built in | Tuner, "Copy config", paste into `firmware/src/config.h`, flash |
| 2. Live over USB | Open a serial monitor, paste config lines, they apply immediately. Later: a "Send to pedal" button in the tuner |
| 3. Presets in flash | Several presets stored in the ESP32 flash, switched with a 4th footswitch. Problem: the pots stay where they are, so they no longer match a loaded preset. Fix: "pickup mode" (a knob takes control only after it is moved), or rotary encoders with an OLED |

---

## 10. Comparing analog and digital

### In software (now)

The tuner already compares the model with the real TS9 recording. Press 2 and 3, read the "tone difference" number and look at the spectrum. See [2b](#2b-how-to-tune).

### On hardware (after phases 2 and 4)

Expected differences at first:

| | Analog TS | Digital model |
|---|---|---|
| Filters | Several RC filters that affect each other | Simple one-pole filters |
| Clipping | Real 1N914 diodes and the 4558's own limits | A math curve, `sat()` |
| Pickup loading | The pedal input loads the pickups | Hidden by the 1 MOhm buffer |
| Aliasing (harsh fizz) | Does not happen | Possible at high drive |
| Noise | Op-amp hiss | Converter noise, possible hum from the ESP32 |
| Consistency | Part tolerances, every unit is a bit different | Identical every time |

How to compare fairly:

- [ ] Match the volume: multimeter on AC volts with a constant note, or by ear. The louder pedal always sounds "better".
- [ ] Reamp: record one dry track. Play it from the PC through a passive DI box (or a 10:1 divider, 10 kOhm / 1 kOhm) into the pedal and record the output. Do the same for both pedals. Start with low volume.
- [ ] Record the analog TS with that same dry track. Name the files `NAME_in.wav` and `NAME_out.wav`. The tuner then uses your pedal as the "Real pedal" reference.
- [ ] Blind test: a friend switches between the pedals without telling you which one is playing.
- [ ] Repeat for the Klon and Katana if you built them.

Possible final setup: the real TS808, then the ESP32 doing Klon and Katana.

---

## 11. Final build

- [ ] Start only when the breadboard version sounds right.
- [ ] Draw the schematic in KiCad: input buffer, power, connectors, module headers, 7 pots, switches.
- [ ] Build on stripboard first, or order a PCB from JLCPCB / PCBWay (about 5 EUR for 5 boards plus shipping).
- [ ] Keep the analog and digital ground areas separate and join them at one point.
- [ ] Drill the 1590XX / 125B: mark, center punch, step drill. Follow the [front-panel layout](#enclosure-buy-after-the-breadboard-version-works).
- [ ] Mount the converter modules on headers so they can be replaced.

---

## 12. Problems and fixes

| Problem | Likely cause |
|---|---|
| No sound from the DAC | XSMT not high, SCK not grounded, or wrong I2S format |
| No sound from the ADC | No MCLK, MD0/MD1 not grounded, or the firmware does not use GPIO 19 for data in |
| ESP32 does not flash | GPIO0 (MCLK) is the boot pin. Disconnect MCLK while flashing |
| Katana VOLUME (GPIO 4) reads random values | ADC2 does not work while Wi-Fi is on. Make sure Wi-Fi is off |
| Knob values jump near the ends | The ESP32 ADC is not linear near 0 V and 3.3 V. Map about 0.1-3.1 V to 0-10 |
| Settings flicker, zipper noise when turning knobs | Pot noise. Average more readings, add hysteresis, check the 100 nF caps |
| Dull sound | Input impedance too low. Check R2 = 1 MOhm and that the buffer works |
| Hum or buzz | Ground loop, long input wires, Wi-Fi on, or a non-isolated power supply |
| Whine that follows ESP32 activity | Digital noise on the power rail. Add decoupling, use a separate analog supply |
| Fizz at high drive | Aliasing. Add oversampling |
| Analog pedal: no sound, no LED | A diode or electrolytic cap is reversed. Check Vref = 4.5 V |
| Klon: weak or distorts too early | The charge pump does not give about 18 V |

### Not verified yet

- The TONE, LEVEL, OUTPUT and VOLUME knob mappings and the Katana dirty mode are approximations. DRIVE uses the real TS808 formula.
- The analog part values in section 5 are from the TS808 schematic from memory. Your kit's BOM is correct.
- No verified Katana schematic was found.
- Shop stock and prices were not checked.
- The firmware is not written yet. ESP32 timing and noise are estimates.

### Ideas for later

- Delay and reverb (needs the WROVER's PSRAM)
- OLED and encoder menu, presets in flash, pickup mode for the pots
- "Send to pedal" button in the tuner (over serial)
- Headphone output with the amp + cab simulation running on the ESP32
- Expression pedal, tap tempo
- More accurate models in `pedal.h` (biquad filters, a real diode equation), fitted to recordings of your analog builds
