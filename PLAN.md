# Magic Mayer Home-Made Pedal: Plan

## Goal

Build guitar effects you can switch on and off:

- **Tube Screamer (TS808/TS9):** overdrive with a mid boost.
- **Klon Centaur:** overdrive that mixes clean and distorted signal.
- **Keeley Katana:** clean boost, with an optional dirty mode.

And a wireless link that replaces the guitar cable. What you learn: analog circuits (buffers, filters, clipping), DSP (the same filters as code), and digital radio.

## 1. Steps

Do them in this order. Each folder has its own README with schematics, parts and build steps.

| Step | Project | What it is | Status |
|---|---|---|---|
| 0 | [Software](#2-software-done) | PC tuner and simulator running `pedal.h` | Done; tune presets |
| 1 | [Wireless guitar system](wireless-guitar-system/README.md) | Battery transmitter on the guitar, receiver at the amp, about 4-5 ms | Next |
| 2 | [Digital pedal](digital-pedal/README.md) | ESP32 box running all three effects from `pedal.h` | Planned |
| 3 | [Analog pedals](analog-pedals/README.md) | Real Tube Screamer (then Klon, Katana) circuits, one box each | Postponed |

Shopping: [SHOPPING_LIST.md](SHOPPING_LIST.md) covers tools, step 1 and step 2, with prices. Step 3 has its own list in [analog-pedals/](analog-pedals/SHOPPING_LIST.md). AliExpress takes 2-3 weeks: order for the next step while building the current one.

```
Wireless:   Guitar -> transmitter (buffer, ADC, radio) ~~ 2.4 GHz ~~> receiver (radio, DAC) -> amp
Digital:    Guitar -> TL072 buffer -> PCM1808 -> ESP32 (pedal.h) -> PCM5102A -> volume -> amp
Analog:     Guitar -> [TS808] -> [Klon]* -> [Katana]* -> amp                       * optional
PC (done):  recording -> tuner / sim (same pedal.h) -> presets/*.cfg -> copied into the firmware
```

## 2. Software (done)

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


### 2e. Tuner controls

- Plays audio through the pedal code in real time. Moving a knob changes the sound immediately.
- Keys **1 / 2 / 3** (or Space) switch between **Dry**, **Model** and **Real pedal** (the real TS9 recording). All three play at the same loudness.
- Each pedal has its real front panel: TS has DRIVE / TONE / LEVEL, Klon has GAIN / TREBLE / OUTPUT, Katana has VOLUME and a pull switch for dirty mode.
  "Circuit internals" holds extra settings for fine-tuning.
- Plots: live waveform, clipping shape, harmonics with THD, frequency response, tone spectrum, and a number that shows how close the model is to the real pedal.
- **Amp + cab** simulates an amp and speaker for listening only. An overdrive is made to drive an amp, so it sounds thin straight into headphones.
- Presets are saved to `presets/*.cfg`. Drag any dry guitar WAV (16/24-bit PCM or 32-bit float) onto the window.
  A file named `*_in.wav` is paired with the matching `*_out.wav`, which is used as the "Real pedal" reference.

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

**Circuit settings** are values inside the pedal that have no knob: `tsHpHz`, `klClip`, `klBpHz`, `klTrebleHz`, `kaBright`, `kaShelfHz`. They can also override what the knobs set. Tune them in the tuner, then copy them into the firmware ([digital pedal, Firmware](digital-pedal/README.md#6-firmware)).

In config files, knobs come first and circuit values after. Setting a knob recalculates its circuit values. Lines after it can override them.


## 4. Comparing analog and digital (after steps 2 and 3)

### In software (now)

The tuner already compares the model with the real TS9 recording. Press 2 and 3, read the "tone difference" number and look at the spectrum. See [2b](#2b-how-to-tune).

### On hardware (after steps 2 and 3)

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


## 5. Parts shared by every step

Tools, power supply and cables are in [SHOPPING_LIST.md](SHOPPING_LIST.md), section 1 and section 6. Minimum: a multimeter, a temperature-controlled soldering iron, a 9 V center-negative pedal power supply, patch cables, and headphones or a small speaker for testing.

## Not verified yet

- Shop stock and most prices were not checked.
- See the "Not verified" section in each project README.

## Ideas for later

- More accurate models in `pedal.h` (biquad filters, a real diode equation), fitted to recordings of your analog builds.
- Possible final setup: guitar -> wireless -> [analog TS808] -> [ESP32 doing Klon + Katana] -> amp.
