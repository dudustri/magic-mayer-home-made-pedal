# Magic Mayer Home-Made Pedal

A home-made guitar pedal with three effects you can switch on and off independently, modelled on the
**Ibanez Tube Screamer (TS808/TS9)**, the **Klon Centaur** and the **Keeley Katana** clean boost.
It runs on an **ESP32** with external audio converters. There's also a desktop app to tune the sound
against a recording of a real Tube Screamer before building any hardware.

The full build plan (parts, wiring, phases, analog reference builds) is in **[PLAN.md](PLAN.md)**.

## Quick start (Linux)

```bash
sudo dnf install SDL2-devel gcc-c++ make git curl   # Fedora; any distro with SDL2 dev headers works
make deps          # fetch Dear ImGui + ImPlot (pinned versions)
make recordings    # optional: real guitar + real TS9 recordings (~63 MB)
make
./tuner            # desktop tuner app
./sim [guitar.wav] [presets/NAME.cfg]   # command line: renders every effect combination to renders/
```

## The tuner

- Plays audio through the pedal code in **real time**: knobs change the sound while it plays.
- **1 / 2 / 3** (or Space) switch between **Dry**, **Model** and **Real pedal** (the real TS9 recording), loudness matched.
- Each pedal has its **real front panel** (TS: DRIVE / TONE / LEVEL, Klon: GAIN / TREBLE / OUTPUT, Katana: VOLUME + pull for dirty boost)
  plus **Circuit internals** for fine-tuning.
- Plots: live waveform, clipping shape, harmonics with THD, small-signal frequency response, tone spectrum, and a model-vs-real tone-match score.
- **Amp + cab** listening aid: an overdrive is made to push an amp, so it sounds thin straight into headphones.
- Presets save to `presets/*.cfg`. Drag any WAV (dry guitar, 16/24-bit PCM or 32-bit float) onto the window.
  A file named `*_in.wav` is paired with a matching `*_out.wav` as the real-pedal reference.

## Layout

| File | What |
|---|---|
| `pedal.h` | **The effects.** Shared by the tuner, `sim` and the ESP32 firmware. Plain C++, no PC-only code |
| `presets/*.cfg` | Settings as `name = value` lines: front-panel knobs first, then circuit overrides |
| `tuner.cpp` | Desktop tuner app (SDL2 + OpenGL + Dear ImGui + ImPlot) |
| `sim.cpp` | Command-line renderer, with self-checks |
| `amp_sim.h` | Amp + cab for listening only, not part of the pedal |
| `guitar_synth.h` | Synthesized guitar solo, used when no recording is available |
| `wav.h` | WAV reading and writing for the PC tools |
| `recordings/` | Real recordings (downloaded by `make recordings`) |
| `PLAN.md` | Hardware plan: parts, wiring, phases |

## Credits

- Recordings: [GuitarML/GuitarLSTM](https://github.com/GuitarML/GuitarLSTM) (GPL-3.0)
- GUI: [Dear ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot) (MIT)
