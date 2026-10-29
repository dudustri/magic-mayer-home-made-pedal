<p align="center"><img src="assets/banner.svg" alt="MMH-MP, Magic Mayer Home-Made Pedal, in retro block letters, with a small cartoon guitarist pulling a funny guitar face" width="100%"></p>

# Magic Mayer Home-Made Pedal (MMH-MP)

Home-made guitar gear, built one step at a time. Three famous overdrive and boost pedals (Tube Screamer, Klon Centaur, Keeley Katana) are recreated, first as code you can hear on the PC, then on a small computer chip, then as real circuits. A wireless link replaces the guitar cable.

| Step | Folder | In one line |
|---|---|---|
| 0 | this folder | PC app: hear and tune the effects before building anything. Done |
| 1 | [wireless-guitar-system/](wireless-guitar-system/README.md) | A small battery box on the guitar sends the sound by radio to a box at the amp |
| 2 | [digital-pedal/](digital-pedal/README.md) | One pedal where an ESP32 chip plays all three effects |
| 3 | [analog-pedals/](analog-pedals/README.md) | The same effects as real circuits, one box each. Postponed |

The full plan is in [PLAN.md](PLAN.md). Everything to buy, with prices, is in [SHOPPING_LIST.md](SHOPPING_LIST.md).

## Run the PC app (Linux)

```bash
sudo dnf install SDL2-devel gcc-c++ make git curl   # Fedora; any distro with SDL2 headers works
make deps && make recordings && make
./tuner            # play, tune and compare with a real Tube Screamer recording
./sim              # render every effect combination to renders/
```

## Other files

| File | Purpose |
|---|---|
| `pedal.h` | The effects. Shared by the PC app and the ESP32 pedal |
| `tuner.cpp`, `sim.cpp`, `amp_sim.h`, `guitar_synth.h`, `wav.h` | PC app and tools |
| `presets/` | Saved sounds |
| `make_schematics.py` | Draws every schematic: `python3 make_schematics.py` |
| `assets/` | Banner (SVG and plain text) |

Credits: recordings from [GuitarML/GuitarLSTM](https://github.com/GuitarML/GuitarLSTM) (GPL-3.0). GUI: [Dear ImGui](https://github.com/ocornut/imgui), [ImPlot](https://github.com/epezent/implot) (MIT).
