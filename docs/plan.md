# Magic Mayer Home-Made Pedal: Plan

## Goal

Build guitar effects you can switch on and off:

- **Tube Screamer (TS808/TS9):** overdrive with a mid boost.
- **Klon Centaur:** overdrive that mixes clean and distorted signal.
- **Keeley Katana:** clean boost, with an optional dirty mode.

And a wireless link that replaces the guitar cable. What you learn: analog circuits (buffers, filters, clipping), DSP (the same filters as code), and digital radio.

## 1. Steps

Do them in this order. Each folder has its own readme with schematics, parts and build steps.

| Step | Project | What it is | Status |
|---|---|---|---|
| 0 | [Tone Lab](../tone-lab/readme.md) | PC app and simulator running `pedal.h` | Done; tune presets |
| 1 | [Wireless guitar system](../wireless-guitar-system/readme.md) | Battery transmitter on the guitar (ESP32-C3 + nRF24), receiver at the amp (ESP32-S3 + nRF24), about 4-5 ms | Next |
| 2 | [Digital pedal](../digital-pedal/readme.md) | The step 1 receiver grows into a pedal: ESP32-S3 running all three effects from `pedal.h`, cable or wireless input | Planned |
| 3 | [Analog pedals](../analog-pedals/readme.md) | Real Tube Screamer (then Klon, Katana) circuits, one box each | Postponed |

Shopping: [shopping-list.md](../shopping-list.md) covers tools, step 1 and step 2, with prices. Step 3 has its own list in [analog-pedals/](../analog-pedals/shopping-list.md). AliExpress takes 2-3 weeks: order for the next step while building the current one.

```
Wireless:   Guitar -> transmitter (buffer, WM8960, ESP32-C3, nRF24) ~~ 2.4 GHz ~~> receiver (nRF24, ESP32-S3, WM8960) -> amp
Digital:    Guitar -> TL072 buffer -> PCM1808 --+
            Wireless -> nRF24 ------------------+-> ESP32-S3 (pedal.h) -> PCM5102A -> volume -> amp
Analog:     Guitar -> [TS808] -> [Klon]* -> [Katana]* -> amp                       * optional
PC (done):  recording -> Tone Lab / sim (same pedal.h) -> presets/*.cfg -> copied into the firmware
```

## 2. Tone Lab (done)

The PC app for hearing and tuning the effects before building anything: [tone-lab/readme.md](../tone-lab/readme.md).

## 3. Controls

Tone Lab and the pedal both use the real front panels with a 0-10 scale. `Pedal::applyPanel()` converts knob positions into circuit values. The sliders in Tone Lab and the pots on the pedal use the same code.

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

**Circuit settings** are values inside the pedal that have no knob: `tsHpHz`, `klClip`, `klBpHz`, `klTrebleHz`, `kaBright`, `kaShelfHz`. They can also override what the knobs set. Tune them in Tone Lab, then copy them into the firmware ([digital pedal, Firmware](../digital-pedal/readme.md#6-firmware)).

In config files, knobs come first and circuit values after. Setting a knob recalculates its circuit values. Lines after it can override them.


## 4. Comparing analog and digital (after steps 2 and 3)

### In software (now)

Tone Lab already compares the model with the real TS9 recording. Press 2 and 3, read the "tone difference" number and look at the spectrum. See [how to tune](../tone-lab/readme.md#how-to-tune).

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
- [ ] Record the analog TS with that same dry track. Name the files `NAME_in.wav` and `NAME_out.wav`. Tone Lab then uses your pedal as the "Real pedal" reference.
- [ ] Blind test: a friend switches between the pedals without telling you which one is playing.
- [ ] Repeat for the Klon and Katana if you built them.

Possible final setup: the real TS808, then the ESP32 doing Klon and Katana.


## 5. Parts shared by every step

Tools, power supply and cables are in [shopping-list.md](../shopping-list.md), section 1 and section 6. Minimum: a multimeter, a temperature-controlled soldering iron, a 9 V center-negative pedal power supply, patch cables, and headphones or a small speaker for testing.

## Not verified yet

- Shop stock and most prices were not checked.
- See the "Not verified" section in each project README.

## Ideas for later

- More accurate models in `pedal.h` (biquad filters, a real diode equation), fitted to recordings of your analog builds.
- Possible final setup: guitar -> wireless -> [analog TS808] -> [ESP32 doing Klon + Katana] -> amp.
