# Digital Pedal (ESP32-S3)

**Step 2 of the project.** One box that runs all three effects (Tube Screamer, Klon, Katana) in software on an ESP32-S3. It takes the guitar from a cable or from the [wireless transmitter](../wireless-guitar-system/readme.md) (an nRF24 radio inside the box). The guitar signal is turned into numbers, [`pedal.h`](../pedal.h) changes them, and they are turned back into sound. The Tone Lab runs the same `pedal.h`, so what you tune on the PC is what the pedal plays. With every effect off, the clean guitar passes through.

![Block diagram: guitar, TL072 buffer, PCM1808 ADC, ESP32-S3 running pedal.h, PCM5102A DAC, master volume, amp. A wireless input through an nRF24 radio also reaches the ESP32-S3. A front panel with 7 pots, a pull switch, 3 footswitches and 3 LEDs feeds the ESP32-S3.](schematics/overview.svg)

Parts and prices: [shopping-list.md](../shopping-list.md), sections 1-7. Knob mapping: [plan, Controls](../docs/plan.md#3-controls).

## Contents

1. [Schematics](#1-schematics)
2. [Parts list](#2-parts-list)
3. [Pin plan](#3-pin-plan)
4. [Cost](#4-cost-approximate)
5. [Build on a breadboard](#5-build-on-a-breadboard-1-weekend)
6. [Firmware](#6-firmware)
7. [Final build](#7-final-build)
8. [Problems and fixes](#8-problems-and-fixes)

## 1. Schematics

Labels with the same name (`9V`, `5V`, `3V3`, `BCK`, `LRCK`, `Vref`, `CE`, `SCK`) are connected. The ESP32-S3 pins are drawn in logical order, not in their order on the board.

### Audio path and power

![Audio path and power. J1 input jack into a TL072 voltage follower biased at 4.5 volt, then C3 into the PCM1808 VINL. PCM1808 SCK, BCK, LRC and OUT to ESP32-S3 GPIO14, 13, 12 and 10. ESP32-S3 GPIO11 to the PCM5102A DIN, with BCK and LRCK shared. nRF24L01+ radio on GPIO39, 40, 41, 42 and 47. PCM5102A LOUT through R5 1 kilohm to the RV8 master volume pot, wiper to the J2 output jack. 9 volt DC jack J3, diode D1, C5 and an AMS1117-5.0 regulator make 5 volt.](schematics/audio-and-power.svg)

The input buffer works like the one in the wireless transmitter, on 9 V instead of 3.3 V: see [how the buffer works](../wireless-guitar-system/readme.md#3-the-input-buffer-explained).

### Front panel

![Front panel. Seven 10 kilohm linear pots between 3V3 and ground, wipers to ESP32-S3 GPIO 1 to 7, each with a 100 nanofarad capacitor to ground. RV7 pull switch to GPIO18. Footswitches SW1, SW2, SW3 to GPIO15, 16 and 17, closing to ground. LEDs on GPIO8, 9 and 21 through 1 kilohm resistors.](schematics/front-panel.svg)

## 2. Parts list

One box: input buffer, converters, ESP32-S3, nRF24 radio, the real front-panel knobs, and three momentary footswitches.

### ESP32-S3, radio and audio converters

| Ref | Qty | Part | Notes |
|---|---|---|---|
| U1 | 1 (+1 spare) | ESP32-S3 DevKitC-1, N16R8 (16 MB flash, 8 MB PSRAM) | The wireless receiver board from step 1. PSRAM allows delay and reverb later |
| U6 | 1 | nRF24L01+ module, with 10 uF + 100 nF (C18, C19) at its power pins | Wireless input. Same module and code as the step 1 receiver |
| U2 | 1 (+1-2 spares) | PCM1808 module (ADC: audio in) | Needs a master clock (MCLK) from the ESP32-S3 |
| U3 | 1 (+1-2 spares) | PCM5102A module (GY-PCM5102, DAC: audio out) | Set the jumpers on the back |
| - | 1 (alternative) | Digilent Pmod I2S2 | Replaces U2 and U3 with one board (DigiKey/Mouser) |

### Input buffer

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| U4 | 1 (+4 spares) | TL072 op-amp (DIP-8) with socket | - | Buffer |
| R1 | 1 | resistor | 1 MOhm | Pull-down on the input jack, prevents pops |
| C1 | 1 | film capacitor | 100 nF | Blocks DC from the guitar |
| R2 | 1 | resistor | 1 MOhm | Biases the op-amp to Vref. With R1 it sets the input impedance: R1 and R2 in parallel = 500 kOhm, like most pedals (R1 = 10 MOhm gives about 1 MOhm) |
| R3, R4 | 2 | resistor | 100 kOhm | Divider, 9 V to 4.5 V (Vref) |
| C2 | 1 | electrolytic | 47 uF, 16 V or more | Keeps Vref stable |
| C3 | 1 | electrolytic | 10 uF, 16 V or more | Coupling cap to the PCM1808 input |
| C4 | 1 | ceramic | 100 nF | Supply decoupling, placed at TL072 pin 8 |
| - | - | - | - | Unused TL072 half: pin 5 to Vref, pin 6 to pin 7. This stops it from oscillating |

### Output

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| RV8 | 1 | pot | 10 kOhm log (A) | Master volume. The PCM5102A outputs about 2.1 Vrms, which is too loud for an amp input. Analog only, the ESP32-S3 does not read it |
| R5 | 1 | resistor | 1 kOhm | Protects the output if it is shorted |

### Power

| Ref | Qty | Part | Value | What it does |
|---|---|---|---|---|
| J3 | 1 | DC jack 2.1 mm | center-negative | 9 V input |
| D1 | 1 | Schottky diode | 1N5817 | Protects against a reversed power supply |
| C5 | 1 | electrolytic | 100 uF, 16 V or more | Smooths the 9 V |
| U5 | 1 (+1-2 spares) | 5 V regulator module | AMS1117-5.0 or 7805 | 9 V to 5 V for the ESP32-S3 and converters |
| C6, C7 | 2 | electrolytic | 10 uF | Regulator input and output (if the module has none) |
| C8-C10 | 3 | ceramic | 100 nF | Decoupling next to U1, U2, U3 |

The TL072 runs directly from 9 V (it needs at least 7 V). The ESP32-S3 board makes its own 3.3 V from the 5 V (5V pin).

### Front panel (the real pedals' controls)

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
| SW1-SW3 | 3 | momentary switch | buttons on the breadboard, soft-touch footswitches in the final build | TS / Klon / Katana on-off. Momentary, not latching: the ESP32-S3 remembers the state |
| LED1-LED3 | 3 | LED 3/5 mm | - | Shows which effects are on |
| R6-R8 | 3 | resistor | 1 kOhm (brighter) to 4.7 kOhm (dimmer) | LED current limit |

All pots are linear (B), even where the real pedal uses a log pot. The code applies the log curve (`Pedal::logTaper`), so the hardware stays simple.

### Connectors and prototyping

| Ref | Qty | Part |
|---|---|---|
| J1, J2 | 2 | 6.35 mm mono jacks (in, out) |
| - | 2 | Breadboards, 830 points |
| - | 1 pack each | Jumper wires male-male and male-female |
| - | 1 | Pin headers (to solder onto the modules) |
| - | 2 | Stripboard or perfboard |
| - | 1 each | Resistor kit and capacitor kit |

### Enclosure (buy after the breadboard version works)

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

### Extra parts for later (optional, cheap now)

| Qty | Part | Allows |
|---|---|---|
| 1-2 | SSD1306 0.96" OLED display (I2C) | Show settings and preset names |
| 2 | EC11 rotary encoder with push button | Change circuit settings and presets on the pedal |
| 1-2 | Extra soft-touch footswitches | Preset switching, tap tempo |
| 1 | 6.35 mm stereo (TRS) jack | Expression pedal or headphone output |

## 3. Pin plan

| Function | ESP32-S3 GPIO | Notes |
|---|---|---|
| I2S MCLK | 14 | 12.288 MHz (256 x 48 kHz) |
| I2S BCK | 13 | Shared by both converters |
| I2S WS / LRCK | 12 | Shared |
| I2S data out to PCM5102A DIN | 11 | |
| I2S data in from PCM1808 OUT | 10 | |
| RV1 TS DRIVE | 1 | ADC1 |
| RV2 TS TONE | 2 | ADC1 |
| RV3 TS LEVEL | 3 | ADC1 |
| RV4 Klon GAIN | 4 | ADC1 |
| RV5 Klon TREBLE | 5 | ADC1 |
| RV6 Klon OUTPUT | 6 | ADC1 |
| RV7 Katana VOLUME | 7 | ADC1. All pots on ADC1, which works while the radio is in use |
| RV7 pull switch (dirty mode) | 18 | To GND, internal pull-up |
| SW1 / SW2 / SW3 | 15 / 16 / 17 | To GND, internal pull-ups, 20-30 ms debounce |
| LED1 / LED2 / LED3 | 8 / 9 / 21 | Through R6-R8 |
| nRF24 CE / CSN / SCK / MOSI / MISO | 39 / 40 / 41 / 42 / 47 | Same pins as the step 1 receiver |
| Do not use | 0, 3, 45, 46 (boot pins), 19, 20 (USB), 26-37 (flash and PSRAM on N16R8), 43, 44 (serial) | |
| Reserved: OLED I2C | 38 / 48 | For later. Check which one drives the board's RGB LED |

The step 1 receiver uses GPIO8 and GPIO9 for the WM8960 setup (I2C). The pedal has no WM8960, so they become LED pins here.

Pot wiring: one outer pin to 3.3 V (never 5 V), the other outer pin to GND, the middle pin to the GPIO, with a 100 nF cap (C11-C17) from the middle pin to GND.

## 4. Cost (approximate)

| Part | Cost |
|---|---|
| Converter modules (with spares) | 10-20 EUR |
| Small parts, TL072, regulator, breadboards, wires | 20-35 EUR |
| Front panel: 6 pots, 1 push/pull pot, buttons, LEDs | 15-25 EUR |
| Jacks, DC jack, footswitches (final) | 15-25 EUR |
| Enclosure and knobs (final) | 20-30 EUR |
| Extra parts (optional) | 15-25 EUR |


## 5. Build on a breadboard (1 weekend)

Build in this order. Test each step before the next one.

1. [ ] Power: 9 V to D1, C5, U5. Measure 5.0 V before connecting anything else.
2. [ ] ESP32-S3 alone: flash a blink program. Never power it from USB and the 5 V rail at the same time.
3. [ ] Output: connect U3 with the jumpers set. Play a 440 Hz sine wave and listen through RV8 with headphones.
4. [ ] Input: connect U2 with MCLK. Pass the input straight to the output. Test with phone audio first.
5. [ ] Buffer: build the TL072 stage, then connect the guitar. The sound must not be dull, hum or distort on hard strums.
6. [ ] Front panel: RV1-RV7 and the pull switch. Print the 0-10 values over serial and turn every knob from end to end.
7. [ ] Footswitches and LEDs: SW1-SW3, LED1-LED3.
8. [ ] Firmware with `pedal.h`: see [section 6](#6-firmware).

### I2S wiring (the ESP32-S3 is the clock master)

| ESP32-S3 | PCM1808 (U2) | PCM5102A (U3) |
|---|---|---|
| GPIO14 (MCLK) | SCK | - (connect the board's SCK to GND: it then uses its internal PLL) |
| GPIO13 (BCK) | BCK | BCK |
| GPIO12 (WS / LRCK) | LRC | LCK |
| GPIO10 (data in) | OUT | - |
| GPIO11 (data out) | - | DIN |
| 5 V / GND | VCC (5 V), VDD (3.3 V) / GND | VIN / GND |

- PCM5102A jumpers on the back: FLT = L, DEMP = L, XSMT = H, FMT = L. If XSMT is not high, there is no sound.
- PCM1808 pins: FMT = GND (I2S format), MD0 = MD1 = GND (slave mode). Use the VINL input, leave VINR unconnected.
- PCM1808 maximum input is about 0.6 x VCC = 3 Vpp at 5 V. A guitar peaks at about 1-2 Vpp, so it fits.

### Input buffer

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

### Output

```
U3 LOUT -- R5 1k -- RV8 (A10K) top pin;  RV8 middle pin -- J2 tip;  RV8 bottom pin -- GND
```

The PCM5102A output has no DC on it, so no coupling capacitor is needed.

### Grounding

- Use one ground point ("star ground") for the 9 V return, regulator, ESP32-S3, converters, radio and buffer.
- Keep the input wires (J1 to buffer to U2) short and away from the ESP32-S3, the radio and the clock wires.
- Wi-Fi and Bluetooth off. The nRF24 is the only radio.
- Keep the nRF24 antenna at the edge of the box, away from the converters. In an aluminum enclosure, mount it behind a plastic window or use the nRF24 version with an external antenna.


## 6. Firmware

A PlatformIO project (Arduino framework, board `esp32-s3-devkitc-1`) in `firmware/`. It includes `../../pedal.h` without changes.

### Structure

| Part | What it does |
|---|---|
| Audio task (core 1) | I2S at 48 kHz, 32-bit slots with 24-bit data, ESP32-S3 as master with MCLK. Loop: read a block of 32-64 samples, convert to float (-1 to +1), run `Pedal::process()` on each sample, convert back, write the block |
| Wireless input | The step 1 receiver code: nRF24 packets at 32 kHz into a jitter buffer, resampled to 48 kHz, then into `Pedal::process()` instead of the PCM1808 input. A switch or a footswitch combination picks cable or wireless |
| Control task (core 0) | Every 10 ms: read the 7 pots (average 8-16 readings each), scale to 0-10, call `set("tsDriveKnob", v)` and so on, but only when a knob moved more than about 0.05 (hysteresis: pot noise does not trigger recalculation). Read the pull switch into `kaDirty`. Debounce the footswitches, toggle `tsOn` / `klonOn` / `katanaOn` on each press, update the LEDs |
| Settings handoff | Settings change between audio blocks (a flag or a short critical section), never in the middle of a block |
| Built-in config | A preset from Tone Lab ("Copy config") is pasted into the code as text and applied at startup with `Pedal::apply()`. It holds the circuit settings. The pots then set the knob values |
| Serial config (stage 2) | Lines sent over USB serial go to `Pedal::apply()`. Same format as `presets/*.cfg`. Allows tuning circuit settings live on the real pedal |

### Steps

1. [ ] Create `firmware/` (PlatformIO, board `esp32dev`, Arduino framework) with the repository root in the include path, for `pedal.h`.
2. [ ] I2S input passed straight to the output, no effects. Check latency (under 3 ms) and noise.
3. [ ] Add `Pedal` with all effects on and fixed settings.
4. [ ] Add pots, footswitches and LEDs.
5. [ ] Add the built-in config from Tone Lab.
6. [ ] Add serial config.
7. [ ] Add 2x oversampling around the clipping stages if harsh high-frequency fizz (aliasing) is audible at high drive.
8. [ ] Measure CPU use: time `process()` per block. Expected: a few percent of one core.

### Moving settings from Tone Lab to the pedal

| Stage | How |
|---|---|
| 1. Built in | Tone Lab, "Copy config", paste into `firmware/src/config.h`, flash |
| 2. Live over USB | Open a serial monitor, paste config lines, they apply immediately. Later: a "Send to pedal" button in Tone Lab |
| 3. Presets in flash | Several presets stored in the ESP32-S3 flash, switched with a 4th footswitch. Problem: the pots stay where they are, so they no longer match a loaded preset. Fix: "pickup mode" (a knob takes control only after it is moved), or rotary encoders with an OLED |


## 7. Final build

- [ ] Start only when the breadboard version sounds right.
- [ ] Draw the schematic in KiCad: input buffer, power, connectors, module headers, 7 pots, switches.
- [ ] Build on stripboard first, or order a PCB from JLCPCB / PCBWay (about 5 EUR for 5 boards plus shipping).
- [ ] Keep the analog and digital ground areas separate and join them at one point.
- [ ] Drill the 1590XX / 125B: mark, center punch, step drill. Follow the [front-panel layout](#enclosure-buy-after-the-breadboard-version-works).
- [ ] Mount the converter modules on headers so they can be replaced.


## 8. Problems and fixes

| Problem | Likely cause |
| No sound from the DAC | XSMT not high, SCK not grounded, or wrong I2S format |
| No sound from the ADC | No MCLK, MD0/MD1 not grounded, or the firmware does not use GPIO 10 for data in |
| ESP32-S3 does not flash | Hold BOOT, press RESET, release BOOT. Check nothing pulls GPIO0, 3, 45 or 46 at power-up |
| Wireless input drops out | See the [wireless problems table](../wireless-guitar-system/readme.md#8-problems-and-fixes). Check the nRF24 antenna is not behind metal |
| Knob values jump near the ends | The ESP32-S3 ADC is not linear near 0 V and 3.3 V. Map about 0.1-3.1 V to 0-10 |
| Settings flicker, zipper noise when turning knobs | Pot noise. Average more readings, add hysteresis, check the 100 nF caps |
| Dull sound | Input impedance too low. Check R2 = 1 MOhm and that the buffer works |
| Hum or buzz | Ground loop, long input wires, Wi-Fi on, or a non-isolated power supply |
| Whine that follows ESP32-S3 activity | Digital noise on the power rail. Add decoupling, use a separate analog supply |
| Fizz at high drive | Aliasing. Add oversampling |

## Not verified yet

- The firmware is not written yet. ESP32-S3 timing and noise are estimates.
- Resampling the 32 kHz radio audio to 48 kHz: CPU cost and sound not tested.
- The TONE, LEVEL, OUTPUT and VOLUME knob mappings and the Katana dirty mode are approximations. DRIVE uses the real TS808 formula.

## Ideas for later

- Delay and reverb (uses the 8 MB PSRAM)
- OLED and encoder menu, presets in flash, pickup mode for the pots
- "Send to pedal" button in Tone Lab (over serial)
- Headphone output with the amp + cab simulation running on the ESP32-S3
- Expression pedal, tap tempo
