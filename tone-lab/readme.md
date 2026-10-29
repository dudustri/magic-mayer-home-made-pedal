# Tone Lab

**Step 0 of the project, done.** A desktop app to hear and tune the three effects (Tube Screamer, Klon, Katana) before building anything. It runs the same [`pedal.h`](../pedal.h) as the digital pedal, so the presets you save here are what the pedal will play. Switch between your dry guitar, the model and a recording of a real Ibanez TS9 while the audio plays, and see how close the model is.

## Run it (Linux)

```bash
cd tone-lab
sudo dnf install SDL2-devel gcc-c++ make git curl   # Fedora; any distro with SDL2 headers works
make deps          # download Dear ImGui and ImPlot (fixed versions)
make recordings    # optional: real guitar and real TS9 recordings (about 63 MB)
make
./tone-lab         # the app
./sim [guitar.wav] [presets/NAME.cfg]   # command line: renders every effect combination to renders/
```

## Files

| File | Purpose |
|---|---|
| [`../pedal.h`](../pedal.h) | The three effects. Used by Tone Lab, by `sim` and later by the ESP32. Every setting has a name and can be loaded from text |
| `tone-lab` | Desktop app. Plays audio through `pedal.h` in real time. Has the real front-panel knobs, extra circuit settings, instant switching between dry / model / real pedal while the audio plays (same loudness, so you hear only the tone difference), an amp + speaker simulation for listening, plots and presets |
| `sim` | Command-line tool. Renders every effect combination to `renders/` and runs self-checks |
| `recordings/` | A real Telecaster recorded dry, and the same signal through a real Ibanez TS9 (drive max, tone and level at noon). From GuitarML |
| `presets/*.cfg` | Saved settings: `default`, and `ts9_max_drive` with the same knobs as the recording |

## Using the app

- Plays audio through the pedal code in real time. Moving a knob changes the sound immediately.
- Keys **1 / 2 / 3** (or Space) switch between **Dry**, **Model** and **Real pedal** (the real TS9 recording). All three play at the same loudness.
- Each pedal has its real front panel: TS has DRIVE / TONE / LEVEL, Klon has GAIN / TREBLE / OUTPUT, Katana has VOLUME and a pull switch for dirty mode.
  "Circuit internals" holds extra settings for fine-tuning.
- Plots: live waveform, clipping shape, harmonics with THD, frequency response, tone spectrum, and a number that shows how close the model is to the real pedal.
- **Amp + cab** simulates an amp and speaker for listening only. An overdrive is made to drive an amp, so it sounds thin straight into headphones.
- Presets are saved to `presets/*.cfg`. Drag any dry guitar WAV (16/24-bit PCM or 32-bit float) onto the window.
  A file named `*_in.wav` is paired with the matching `*_out.wav`, which is used as the "Real pedal" reference.

## How to tune

- [ ] Load the `ts9_max_drive` preset. Press 2 and 3 to switch between the model and the real pedal. Change the TS circuit settings (mid hump, tone corner, gain) until the "tone difference" number goes down.
- [ ] Match the compression of the real TS9: its output level changes about 6 dB while the input changes about 15 dB.
- [ ] Check the harmonics plot. A symmetric clipper like the TS makes mostly odd harmonics (3rd, 5th).
- [ ] Record your own guitar dry (any audio interface, 16 or 24-bit WAV) and drag it in. Different pickups change the result a lot.
- [ ] Save one preset per sound, for example `ts-lead`, `klon-push`, `katana-clean`.
- [ ] Turn "Amp + cab" off sometimes. The ESP32 outputs only the pedal, so the sound must work without the amp simulation.

## Analog circuit simulation (free, to learn the TS circuit)

| Tool | Use |
|---|---|
| Falstad (falstad.com/circuit, in the browser) | Quick and visual. Shows the signal and the clipping live |
| KiCad + ngspice (`sudo dnf install kicad`) | Frequency-response plots. The same tool can later make a PCB |
| LTspice (Windows/Mac, or Wine on Linux) | Many ready-made Tube Screamer files exist online |

- [ ] Draw the TS808 clipping stage: op-amp, 4.7 kOhm + 47 nF to Vref, 51 kOhm + 500 kOhm drive pot and 51 pF in the feedback loop, two 1N914 diodes in the feedback loop.
- [ ] Find the high-pass corner: f = 1 / (2 * pi * 4.7 kOhm * 47 nF) = about 720 Hz. In code this is `tsHpHz`.
- [ ] Check the gain formula of the DRIVE knob: 1 + (51 kOhm + R_drive) / 4.7 kOhm = 12x to 118x. In code this is `applyPanel()`.

## Testing the controls without hardware (optional)

- [ ] Wokwi (wokwi.com) simulates an ESP32 with pots, buttons and LEDs in the browser. It cannot simulate the I2S audio.

## Credits

- Recordings: [GuitarML/GuitarLSTM](https://github.com/GuitarML/GuitarLSTM) (GPL-3.0), see [recordings/readme.md](recordings/readme.md)
- GUI: [Dear ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot) (MIT)
