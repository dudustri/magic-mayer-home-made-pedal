# Magic Mayer Home-Made Pedal

A home-made guitar pedal with three effects. Each effect can be switched on or off on its own:

- **Tube Screamer (TS808/TS9):** overdrive with a mid boost
- **Klon Centaur:** overdrive that mixes clean and distorted signal
- **Keeley Katana:** clean boost, with an optional dirty mode

The pedal runs on an **ESP32** with external audio converters. A desktop app lets you tune the sound on a PC,
and compare it with a recording of a real Tube Screamer, before building any hardware.

The full hardware plan (parts, wiring, build steps) is in [PLAN.md](PLAN.md).

## Quick start (Linux)

```bash
sudo dnf install SDL2-devel gcc-c++ make git curl   # Fedora. Any distro with SDL2 development headers works
make deps          # download Dear ImGui and ImPlot (fixed versions)
make recordings    # optional: real guitar and real TS9 recordings (about 63 MB)
make
./tuner            # desktop tuner app
./sim [guitar.wav] [presets/NAME.cfg]   # command line: renders every effect combination to renders/
```

## The tuner

- Plays audio through the pedal code in real time. Moving a knob changes the sound immediately.
- Keys **1 / 2 / 3** (or Space) switch between **Dry**, **Model** and **Real pedal** (the real TS9 recording). All three play at the same loudness.
- Each pedal has its real front panel: TS has DRIVE / TONE / LEVEL, Klon has GAIN / TREBLE / OUTPUT, Katana has VOLUME and a pull switch for dirty mode.
  "Circuit internals" holds extra settings for fine-tuning.
- Plots: live waveform, clipping shape, harmonics with THD, frequency response, tone spectrum, and a number that shows how close the model is to the real pedal.
- **Amp + cab** simulates an amp and speaker for listening only. An overdrive is made to drive an amp, so it sounds thin straight into headphones.
- Presets are saved to `presets/*.cfg`. Drag any dry guitar WAV (16/24-bit PCM or 32-bit float) onto the window.
  A file named `*_in.wav` is paired with the matching `*_out.wav`, which is used as the "Real pedal" reference.

## Files

| File | Purpose |
|---|---|
| `pedal.h` | The effects. Used by the tuner, by `sim` and by the ESP32 firmware. Plain C++ with nothing PC-specific |
| `presets/*.cfg` | Settings as `name = value` lines. Front-panel knobs first, then circuit overrides |
| `tuner.cpp` | Desktop tuner app (SDL2, OpenGL, Dear ImGui, ImPlot) |
| `sim.cpp` | Command-line renderer with self-checks |
| `amp_sim.h` | Amp and speaker simulation for listening. Not part of the pedal |
| `guitar_synth.h` | Synthesized guitar solo, used when no recording is available |
| `wav.h` | WAV file reading and writing for the PC tools |
| `recordings/` | Real recordings, downloaded with `make recordings` |
| `PLAN.md` | Hardware plan: parts, wiring, build steps |

## Credits

- Recordings: [GuitarML/GuitarLSTM](https://github.com/GuitarML/GuitarLSTM) (GPL-3.0)
- GUI: [Dear ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot) (MIT)
