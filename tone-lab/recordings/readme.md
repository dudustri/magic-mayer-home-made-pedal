# Recordings

`ts9_in.wav` / `ts9_out.wav` come from [GuitarML/GuitarLSTM](https://github.com/GuitarML/GuitarLSTM) (`data/ts9_test1_*_FP32.wav`), GPL-3.0.

- `ts9_in.wav`: Fender Telecaster, bridge pickup, max tone and volume, recorded dry (DI).
- `ts9_out.wav`: the same signal split (JHS Buffer Splitter) into a real Ibanez TS9 Tube Screamer: max drive, tone and level at noon.

Both are mono, 32-bit float, 44.1 kHz, sample-aligned. Tone Lab loads `*_in.wav` and pairs it with `*_out.wav` automatically.
Personal use here; keep this attribution if you share them.
