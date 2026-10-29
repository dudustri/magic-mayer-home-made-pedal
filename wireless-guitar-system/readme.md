# Wireless Guitar System

**Step 1 of the project.**

A cable replacement for the guitar. A small battery **transmitter** plugs straight into the guitar. The **receiver** sits at the amp, PC or headphones and has both a 3.5 mm jack ("P3") and a 6.35 mm jack ("P10"). The receiver uses an ESP32-S3, which later becomes the [digital pedal](../digital-pedal/readme.md): the same board receives the guitar by radio and plays the effects.

![Block diagram. Transmitter: guitar, MCP6002 buffer, WM8960 ADC, XIAO ESP32-C3 with nRF24 radio. 2.4 GHz link, about 4 to 5 ms. Receiver: ESP32-S3 with nRF24 radio, WM8960 DAC and headphone amp, 3.5 mm and 6.35 mm jacks.](schematics/system-overview.svg)

| | |
|---|---|
| Latency | About 4-5 ms (estimate), like standing 1.5 m from the amp |
| Audio | 32 kHz, 16-bit: sound up to 16 kHz, enough for guitar |
| Transmitter battery | About 7 h on 400 mAh, about 10 h on 600 mAh (estimate). Charged over USB-C |
| Receiver power | USB-C: power bank, charger or PC. Inside the pedal later: the pedal's 9 V |
| Range | About 10 m indoors (estimate) |
| Parts cost | About 50 EUR for both sides, without spares or tools |
| Software | Arduino / PlatformIO and the `RF24` library on both sides, like the pedal |
| Parts list | [shopping-list.md](../shopping-list.md), section 8 |

Want it working today instead? A ready-made 2.4/5.8 GHz set costs 25-40 EUR on AliExpress (search "guitar wireless system 5.8GHz").

## Contents

1. [The design](#1-the-design)
2. [How the signal travels](#2-how-the-signal-travels)
3. [The input buffer, explained](#3-the-input-buffer-explained)
4. [Schematics](#4-schematics)
5. [Parts](#5-parts)
6. [Build](#6-build)
7. [Firmware](#7-firmware)
8. [Problems and fixes](#8-problems-and-fixes)
9. [Alternative: nRF52840 for a longer battery life](#9-alternative-nrf52840-for-a-longer-battery-life)
10. [Not verified yet](#10-not-verified-yet)

---

## 1. The design

| | Transmitter (on the guitar) | Receiver (at the amp) |
|---|---|---|
| Processor | Seeed XIAO ESP32-C3: tiny, has a LiPo charger | ESP32-S3 DevKitC-1 (N16R8): powerful, runs the effects later |
| Radio | nRF24L01+ mini module (SPI) | nRF24L01+ module (SPI) |
| Audio | MCP6002 buffer + WM8960 (ADC) | WM8960 (DAC + headphone amp) |
| Connector | 6.35 mm male plug, straight into the guitar | 3.5 mm and 6.35 mm jacks, same signal on both |
| Power | LiPo 1S 400-600 mAh, slide switch | USB-C |
| Box | Small 3D-printed case, only the plug sticks out | Any box. Later: the pedal enclosure |

Why this design:

- **One toolset.** Both sides and the pedal use Arduino / PlatformIO. No second programming environment to learn.
- **The radio does one job.** The nRF24 sends raw packets in under 1 ms. The ESP32 Wi-Fi radios stay off: less power, less noise.
- **The receiver grows into the pedal.** The ESP32-S3 has the power and memory for the effects (and later delay and reverb). Its receiver code is reused in the pedal unchanged.
- **Clean sound.** With every effect off, the pedal passes the clean guitar through.

### Why not Bluetooth?

Distance is not what makes a link slow. Radio covers 10 m in about 0.00003 ms. The delay comes from how the protocol schedules and packs the audio:

| Link | Delay |
|---|---|
| Bluetooth headphones (A2DP) | 100-200 ms |
| Bluetooth LE Audio | 20-40 ms |
| Bluetooth LE, own data | At least 7.5 ms per message, the shortest interval BLE allows |
| nRF24L01+ (this design) | Under 1 ms per packet |

"Almost instant" Bluetooth usually means a short connection interval, at its best 7.5 ms just for the radio. The nRF24L01+ sends raw packets with no fixed interval.

## 2. How the signal travels

1. **Buffer:** the MCP6002 copies the guitar signal without loading the pickups ([section 3](#3-the-input-buffer-explained)).
2. **ADC:** the WM8960 turns it into numbers, 32,000 per second.
3. **Radio:** the ESP32-C3 packs 15 numbers (0.47 ms of sound) plus a packet number into one 32-byte nRF24 packet and sends it. About 2,100 packets per second. No resends: a resend would arrive too late.
4. **Jitter buffer:** the receiver keeps 4 packets (about 2 ms) in stock, so a late packet does not click. A lost packet is replaced by the previous one, faded out.
5. **DAC:** the receiver's WM8960 turns the numbers back into sound. Its headphone amp drives both jacks.

| Step | Time |
|---|---|
| Fill one packet | 0.5 ms |
| Radio | About 0.3 ms |
| Jitter buffer, 4 packets | 1.9 ms |
| Converter buffers | About 1-2 ms |
| **Total** | **About 4-5 ms** |

## 3. The input buffer, explained

![Input buffer schematic with three measuring points. Point 1 at the guitar tip: a wave of about plus and minus 0.5 volt around 0 volt. Point 2 after C1 and R2: the same wave around 1.65 volt. Point 3 after C3: the same wave around 0 volt again, going to the WM8960.](schematics/buffer-explained.svg)

**Why a buffer is needed.** A guitar pickup is a weak signal source. Connected straight to the WM8960 (input resistance a few kOhm to tens of kOhm), the pickup gets loaded and loses its treble: the sound gets dull. The buffer has a very high input resistance, so the pickup "sees" almost nothing, and a strong output that drives the WM8960 easily.

**What a voltage follower is.** U3A is an op-amp whose output (pin 1) is wired straight back to its minus input (pin 2). An op-amp always tries to make its two inputs equal. With the output tied to the minus input, the output simply copies the plus input (pin 3): gain 1, same wave. The difference is strength: the input takes almost no current, the output can supply plenty.

**Why the 1.65 V.** The op-amp runs on 3.3 V and ground only. It cannot output a negative voltage, but the guitar wave goes below 0 V half the time. So the wave is lifted to the middle of the supply:

| Part | What it does |
|---|---|
| C1 100 nF | Passes the wave, blocks DC. The guitar never sees the 1.65 V |
| R2 1 MOhm | Pulls the wave's center to Vref = 1.65 V (point 2). Its high value keeps the input "light" |
| R1 1 MOhm | Keeps the input at 0 V when nothing is plugged in. No pop when plugging in |
| R3, R4 100 kOhm | Divide 3.3 V in half: Vref = 1.65 V |
| C2 10 uF | Keeps Vref steady and quiet |
| C3 1 uF | Removes the 1.65 V again before the WM8960 (point 3) |
| C4 100 nF | Supply filter, placed right at U3 pin 8 |
| U3B | The second op-amp in the chip is not needed. It is wired as a follower on Vref so it stays quiet |

The guitar sees R1 and R2 in parallel: 500 kOhm, the same as most overdrive pedals. For about 1 MOhm, change R1 to 10 MOhm.

## 4. Schematics

Labels with the same name (`3V3`, `Vref`, `CE`, `SCK`) are connected. The XIAO pins are drawn in their real order; the ESP32-S3 pins in logical order.

### Transmitter

![Transmitter schematic. Guitar plug, MCP6002 buffer and C3 into WM8960 LINPUT1. WM8960 MCLK, BCLK, LRCK, ADCDAT, SDA, SCL to XIAO ESP32-C3 D0 to D5. nRF24L01+: D10 MOSI, D9 MISO, D8 SCK, D7 CSN with a 10 kilohm pull-up, D6 CE, with 10 microfarad and 100 nanofarad on its power pins. LiPo through switch SW1 to the XIAO battery pads.](schematics/tx.svg)

### Receiver

![Receiver schematic. ESP32-S3 GPIO14, 13, 12, 11, 8, 9 to WM8960 MCLK, BCLK, LRCK, DACDAT, SDA, SCL. nRF24L01+ on GPIO39 CE, 40 CSN, 41 SCK, 42 MOSI, 47 MISO. WM8960 HP_L and HP_R through 47 ohm to the 3.5 mm jack; HP_L also to the 6.35 mm jack. Powered over USB-C.](schematics/rx.svg)

### Connection list

| Signal | Transmitter: XIAO ESP32-C3 | Receiver: ESP32-S3 | Goes to |
|---|---|---|---|
| I2S master clock (MCLK) | D0 | GPIO14 | WM8960 MCLK |
| I2S bit clock | D1 | GPIO13 | WM8960 BCLK |
| I2S word clock | D2 | GPIO12 | WM8960 LRCK (ADCLRC / DACLRC) |
| Audio data | D3 (from ADCDAT) | GPIO11 (to DACDAT) | WM8960 |
| I2C data / clock | D4 / D5 | GPIO8 / GPIO9 | WM8960 SDA / SCL (setup only) |
| nRF24 CE | D6 | GPIO39 | nRF24 CE |
| nRF24 CSN | D7, 10 kOhm pull-up to 3V3 | GPIO40 | nRF24 CSN |
| SPI clock | D8 | GPIO41 | nRF24 SCK |
| SPI MISO | D9 | GPIO47 | nRF24 MISO |
| SPI MOSI | D10 | GPIO42 | nRF24 MOSI |
| 3.3 V | 3V3 | 3V3 | WM8960 VCC, nRF24 VCC (+10 uF and 100 nF at its pins), MCP6002 |
| Power in | LiPo on the BAT pads, through SW1 | USB-C | |

The 10 kOhm pull-up on the transmitter's CSN keeps the nRF24 quiet while the ESP32-C3 starts: D9 is a boot pin, and the nRF24 must not pull it low.

## 5. Parts

Buy links, spares and prices: [shopping-list.md](../shopping-list.md), section 8.

| Ref | Part | Transmitter | Receiver |
|---|---|---|---|
| U1 | XIAO ESP32-C3 / ESP32-S3 DevKitC-1 N16R8 | 1 | 1 |
| U2 | WM8960 module | 1 | 1 |
| U3 | MCP6002 (DIP-8 on breadboard, SOIC-8 on a PCB) | 1 | - |
| U4 | nRF24L01+ (mini SMD in the transmitter) | 1 | 1 |
| R1, R2 | 1 MOhm (transmitter) / 47 Ohm (receiver) | 2 | 2 |
| R3, R4 | 100 kOhm | 2 | - |
| R6 | 10 kOhm, CSN pull-up | 1 | - |
| C1 | 100 nF film or C0G ceramic | 1 | - |
| C2 | 10 uF | 1 | - |
| C3 | 1 uF | 1 | - |
| C4 | 100 nF ceramic at U3 | 1 | - |
| C6, C7 | 10 uF + 100 nF at the nRF24 | 1 each | 1 each |
| P1 / J2, J3 | 6.35 mm mono male plug / 6.35 mm mono jack + 3.5 mm stereo jack | 1 | 1 + 1 |
| SW1 | Mini slide switch | 1 | - |
| BT1 | LiPo 1S 400-600 mAh with protection board, JST-PH | 1 | - |

## 6. Build

### Stage 1: breadboard

1. Solder pin headers on the XIAO, the WM8960 modules and the nRF24 modules (or use the 8-pin nRF24 breadboard adapter).
2. Radio first: one nRF24 on each board, send numbered packets, count the lost ones ([section 7](#7-firmware), step 2).
3. Receiver audio: ESP32-S3 + WM8960, play a test tone on both jacks.
4. Transmitter buffer: measure 1.65 V at Vref and at U3 pin 1 with nothing plugged in.
5. Join both sides: phone audio first, then the guitar.

### Stage 2: soldered boards

- **Transmitter:** hand-wired double-sided prototype board cut to the case, boards stacked (buffer board, WM8960, XIAO), the mini nRF24 at the end of the case away from the battery. A custom PCB (KiCad, JLCPCB, about 5 USD for 5 boards) is the smallest option.
- **Receiver:** a perfboard with headers in any box, powered over USB-C. In step 2 its parts move into the pedal.
- Check every connection with the multimeter (continuity) before powering up.

### The transmitter case (3D printed)

- Openings: the plug, USB-C (charging and flashing), the slide switch, and a thin wall or small hole over the LED.
- No metal near the nRF24 antenna (the zigzag track at the end of the module). No metallic paint or foil.
- The plug carries the whole box. Clamp its metal barrel between the case halves, or screw it into a printed boss with its nut. Close with M2 screws in heat-set inserts.
- PETG is stronger than PLA and survives a hot car.
- Guitar jacks differ: recessed (Stratocaster), on the side (Les Paul), in a cup (Telecaster). Print the plug end first and try it on your guitar. Use the right-angle plug if a straight case hits the body.
- Pad the battery and keep it away from the plug, so a fall cannot puncture it.
- The slide switch cuts the battery, so the battery charges only with the switch on.

## 7. Firmware

PlatformIO, Arduino framework. Boards `seeed_xiao_esp32c3` and `esp32-s3-devkitc-1`. Library `RF24`. Later in `firmware/tx` and `firmware/rx`; the receiver code moves into the pedal firmware.

| Part | Transmitter | Receiver |
|---|---|---|
| WM8960 setup (I2C) | Line input, PGA gain, ADC on, 32 kHz, slave | DAC on, headphone amp on, 32 kHz, slave |
| I2S | ESP32-C3 master, MCLK 8.192 MHz (256 x 32 kHz), read 15 samples | ESP32-S3 master, same clocks, write each sample to left and right |
| Radio | nRF24 at 2 Mbit/s, fixed channel, auto-acknowledge off, 32-byte payload: 2-byte packet number + 15 samples | Same settings, receive mode, ring buffer by packet number |
| Lost packet | - | Repeat the last packet with a fade-out, then silence |
| Clock drift | - | The two crystals differ slightly (about 1 sample per second). Drop or repeat one sample when the jitter buffer drifts |
| Wi-Fi / Bluetooth | Off | Off |

1. [ ] Blink test on both boards.
2. [ ] Radio only: send 2,000 numbered packets per second, count the lost ones at 1, 5 and 10 m, with your body in between. Try channels away from your Wi-Fi.
3. [ ] Receiver: WM8960 setup over I2C, play a 440 Hz tone on headphones and into the amp.
4. [ ] Transmitter: WM8960 setup, print the input level over USB serial with phone audio as input.
5. [ ] Both: phone audio in, headphones out. Then the guitar.
6. [ ] Measure the latency: dry guitar on one input of an audio interface, receiver output on the other. Read the delay in Audacity.
7. [ ] Tune the jitter buffer size and the radio channel.
8. [ ] Battery voltage over serial; later a low-battery blink.

## 8. Problems and fixes

| Problem | Likely cause |
|---|---|
| Radio works sometimes | Missing 10 uF / 100 nF at the nRF24, long wires, or a fake chip. Try another module |
| Many lost packets | Busy channel (try 2.4xx MHz channels above your Wi-Fi), body between the boxes, antenna near metal |
| Clicks every few seconds | Jitter buffer too small, or drift correction not working |
| Transmitter does not start | nRF24 pulling D9 (a boot pin) low. Check the R6 pull-up on CSN |
| Dull sound | Buffer missing or bypassed, or R2 is not 1 MOhm |
| Hiss | WM8960 input gain too high. A hard strum should reach about -6 dBFS |
| No sound at all | WM8960 not set up (check I2C), no MCLK, or wrong I2S master/slave setting |
| Battery does not charge | Slide switch off (it cuts the battery) |
| Hum at the amp | USB charger ground loop. Use a power bank for the receiver |

## 9. Alternative: nRF52840 for a longer battery life

A Seeed XIAO nRF52840 does processor and radio in one chip at 15-30 mA: 13-20 h of transmitter battery and 48 kHz audio with resends, at the cost of learning the nRF Connect SDK (Zephyr). The schematics stay in this folder: [transmitter](schematics/tx-nrf52840.svg), [receiver](schematics/rx-nrf52840.svg). Its radio can be set to the nRF24-compatible mode, so a later nRF52840 transmitter should still work with this receiver. Parts: [shopping-list.md](../shopping-list.md), section 9.

## 10. Not verified yet

- The WM8960 module: size, pin names and how it grounds its headphone output vary by seller. If its headphone ground is "OUT3" (capless mode), do not connect it to GND: add 220 uF caps in series with HP_L and HP_R and use GND for the sleeve.
- MCLK accuracy: the ESP32-C3 and ESP32-S3 make it with a fractional divider. Probably fine for guitar; check that the WM8960 accepts it.
- 32 kHz through the nRF24 without resends: the throughput is close to the nRF24's practical limit. Test step 2 first.
- XIAO pin order: check against the Seeed pinout before soldering.
- Latency, range and battery life are estimates.

Sources: [Seeed XIAO ESP32-C3 wiki](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/), [SparkFun WM8960 hookup guide](https://www.digikey.com/en/maker/projects/audio-codec-breakout-wm8960-hookup-guide/64b54a59f8004b81ad76ab21cb7f2c65).
