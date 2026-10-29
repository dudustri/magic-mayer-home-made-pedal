# Shopping List (AliExpress)

Every part for the digital pedal (B) and the [wireless guitar system](wireless-guitar-system/readme.md) (W), plus tools. The analog pedals (Branch A) are postponed: their list is in [analog-pedals/shopping-list.md](analog-pedals/shopping-list.md).
The "Buy" column already includes spares. Phase 0 (software) needs nothing.

**For:** A = analog pedals, B = digital pedal, W = wireless system, all = shared.
"opt" in the For column = only if you build that optional part.

## Where to buy

AliExpress has everything below. Two cheaper or better shops for some of it:

| Shop | Good for | Not good for |
|---|---|---|
| **Tayda Electronics** (taydaelectronics.com) | Made for DIY pedal builders: 1590 enclosures (with a drilling and printing service), pots, 3PDT switches, jacks, DC jacks, knobs, LEDs and bezels, resistors, capacitors, op-amps. Low prices, real parts | ESP32 boards and audio modules |
| **LCSC** (lcsc.com) | Real ICs and passives at factory prices: TL072, NJM4558D, diodes, capacitors | Modules, pedal hardware, kits |
| **AliExpress** | Modules (ESP32, PCM1808, PCM5102A), kits, tools, cables | Op-amps and germanium diodes: fakes are common |

Recommendation: modules, kits and tools from AliExpress. Pedal hardware and small parts from Tayda if you want better quality for about the same price. If you want one shop only, AliExpress works.

How to pick a listing (each link opens an AliExpress search sorted by number of orders):
- Rating 4.7 or higher, at least a few hundred sold, store with 95% or more positive feedback.
- Read reviews with photos: the part in the photo must match the title (for example a real "NJM4558D" print, a PCM5102A board with the jumpers on the back).
- Prefer official brand stores: Seeed Studio (XIAO boards), PINE64 (Pinecil), ANENG (multimeters), Joyo / Donner (power supplies). The search shows the store name under each item.
- "Choice" items ship faster and are easier to return.

Tips:
- Buy from stores with many orders and good ratings. For the XIAO, use the Seeed Studio official store.
- Orders from outside the EU: VAT is added at checkout. Orders above 150 EUR can also pay customs duty. Split large orders.
- Mains power supplies: buy a known brand (Joyo, Donner, Caline) with CE marking.
- Test every module when it arrives. Open a dispute within the protection period if one is dead.

---

## 1. Tools (buy once)

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| Multimeter | [`ANENG multimeter`](https://www.aliexpress.com/w/wholesale-ANENG-multimeter.html?SortType=total_tranpro_desc) | 1 | all | DC volts, AC volts, continuity, diode test |
| Soldering iron | [`Pinecil V2`](https://www.aliexpress.com/w/wholesale-Pinecil-V2.html?SortType=total_tranpro_desc) | 1 | all | Needs a USB-C PD charger, 65 W. A T12 station also works |
| Spare iron tip | [`Pinecil tip`](https://www.aliexpress.com/w/wholesale-Pinecil-tip.html?SortType=total_tranpro_desc) (or for your iron) | 1 | all | Conical or small chisel |
| Solder 0.8 mm, 63/37, flux core | [`solder wire 63/37 0.8mm`](https://www.aliexpress.com/w/wholesale-solder-wire-63%2F37-0.8mm.html?SortType=total_tranpro_desc) | 1 roll (100 g) | all | |
| Flux | [`flux paste soldering`](https://www.aliexpress.com/w/wholesale-flux-paste-soldering.html?SortType=total_tranpro_desc) | 1 | all | |
| Brass tip cleaner | [`brass soldering tip cleaner`](https://www.aliexpress.com/w/wholesale-brass-soldering-tip-cleaner.html?SortType=total_tranpro_desc) | 1 | all | |
| Desoldering pump | [`desoldering pump`](https://www.aliexpress.com/w/wholesale-desoldering-pump.html?SortType=total_tranpro_desc) | 1 | all | |
| Desoldering wick 2 mm | [`desoldering wick`](https://www.aliexpress.com/w/wholesale-desoldering-wick.html?SortType=total_tranpro_desc) | 2 | all | |
| Flush side cutters | [`flush cutter`](https://www.aliexpress.com/w/wholesale-flush-cutter.html?SortType=total_tranpro_desc) | 1 | all | |
| Wire stripper | [`wire stripper`](https://www.aliexpress.com/w/wholesale-wire-stripper.html?SortType=total_tranpro_desc) | 1 | all | |
| Tweezers set | [`ESD tweezers set`](https://www.aliexpress.com/w/wholesale-ESD-tweezers-set.html?SortType=total_tranpro_desc) | 1 | all | |
| PCB holder (helping hands) | [`PCB holder soldering`](https://www.aliexpress.com/w/wholesale-PCB-holder-soldering.html?SortType=total_tranpro_desc) | 1 | all | |
| Step drill bit set (4-32 mm) | [`step drill bit set HSS`](https://www.aliexpress.com/w/wholesale-step-drill-bit-set-HSS.html?SortType=total_tranpro_desc) | 1 | A, B, W | You also need a drill |
| Automatic center punch | [`automatic center punch`](https://www.aliexpress.com/w/wholesale-automatic-center-punch.html?SortType=total_tranpro_desc) | 1 | A, B, W | |
| Small nut driver / socket set | [`nut driver set`](https://www.aliexpress.com/w/wholesale-nut-driver-set.html?SortType=total_tranpro_desc) | 1 | A, B, W | For jack and pot nuts |
| Safety glasses | [`safety glasses`](https://www.aliexpress.com/w/wholesale-safety-glasses.html?SortType=total_tranpro_desc) | 1 | all | For drilling |
| Alligator clip test leads | [`alligator clip test leads`](https://www.aliexpress.com/w/wholesale-alligator-clip-test-leads.html?SortType=total_tranpro_desc) | 1 pack (10) | B, W | Connects jacks and the multimeter to the breadboard |
| USB-C data cable | [`USB C data cable`](https://www.aliexpress.com/w/wholesale-USB-C-data-cable.html?SortType=total_tranpro_desc) | 3 | B, W | ESP32-S3 and XIAO: flashing, receiver power, battery charging. Must be a data cable, not charge-only |

Not on AliExpress or probably already at home: a drill, isopropyl alcohol, headphones or a small powered speaker.

## 2. Wire and prototyping

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| Solid-core wire 22 AWG, 6 colors | [`22AWG solid wire kit`](https://www.aliexpress.com/w/wholesale-22AWG-solid-wire-kit.html?SortType=total_tranpro_desc) | 1 | all | Breadboard and board jumpers |
| Stranded silicone wire 24 AWG, several colors | [`24AWG silicone wire kit`](https://www.aliexpress.com/w/wholesale-24AWG-silicone-wire-kit.html?SortType=total_tranpro_desc) | 1 | all | Jacks, switches and pots in the enclosure |
| Heat-shrink tubing set | [`heat shrink tube kit`](https://www.aliexpress.com/w/wholesale-heat-shrink-tube-kit.html?SortType=total_tranpro_desc) | 1 | all | |
| Breadboard, 830 points | [`breadboard 830`](https://www.aliexpress.com/w/wholesale-breadboard-830.html?SortType=total_tranpro_desc) | 4 | B, W | The ESP32-S3 DevKit needs two side by side. One each for the wireless transmitter and receiver |
| Jumper wires M-M, M-F, F-F | [`dupont jumper wire 20cm`](https://www.aliexpress.com/w/wholesale-dupont-jumper-wire-20cm.html?SortType=total_tranpro_desc) | 1 pack each | B, W | |
| Male pin headers 2.54 mm, 40 pin | [`pin header 2.54 40pin male`](https://www.aliexpress.com/w/wholesale-pin-header-2.54-40pin-male.html?SortType=total_tranpro_desc) | 10 | B, W | |
| Female pin headers 2.54 mm, 40 pin | [`pin header 2.54 40pin female`](https://www.aliexpress.com/w/wholesale-pin-header-2.54-40pin-female.html?SortType=total_tranpro_desc) | 10 | B, W | Modules plug in, so they can be replaced |
| Perfboard / stripboard | [`stripboard`](https://www.aliexpress.com/w/wholesale-stripboard.html?SortType=total_tranpro_desc) or [`perfboard 7x9`](https://www.aliexpress.com/w/wholesale-perfboard-7x9.html?SortType=total_tranpro_desc) | 5 | B, W | |
| DC jack to screw terminal adapter | [`DC 5.5x2.1 female screw terminal`](https://www.aliexpress.com/w/wholesale-DC-5.5x2.1-female-screw-terminal.html?SortType=total_tranpro_desc) | 2 | B | Breadboard power. Pedal supplies are center-negative: the "+" screw is then GND |
| M3 nylon standoff kit | [`M3 nylon standoff kit`](https://www.aliexpress.com/w/wholesale-M3-nylon-standoff-kit.html?SortType=total_tranpro_desc) | 1 | B, W | Mounts boards in the enclosure |
| Double-sided foam tape | [`double sided foam tape`](https://www.aliexpress.com/w/wholesale-double-sided-foam-tape.html?SortType=total_tranpro_desc) | 1 | B, W | |

## 3. Small components

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| Resistor kit, 1/4 W metal film 1%, 600+ pcs | [`resistor kit 1/4W metal film`](https://www.aliexpress.com/w/wholesale-resistor-kit-1%2F4W-metal-film.html?SortType=total_tranpro_desc) | 1 | all | Must include 1 M, 100 k, 10 k, 4.7 k, 2.2 k, 1 k |
| Ceramic capacitor kit | [`ceramic capacitor kit`](https://www.aliexpress.com/w/wholesale-ceramic-capacitor-kit.html?SortType=total_tranpro_desc) | 1 | all | |
| Extra 100 nF ceramic capacitors | [`100nF ceramic capacitor 50V`](https://www.aliexpress.com/w/wholesale-100nF-ceramic-capacitor-50V.html?SortType=total_tranpro_desc) | 50 | B, W | About 25 are used |
| Electrolytic capacitor kit (0.22-470 uF) | [`electrolytic capacitor kit`](https://www.aliexpress.com/w/wholesale-electrolytic-capacitor-kit.html?SortType=total_tranpro_desc) | 1 | all | Must include 10, 47, 100 uF at 16 V or more |
| 100 nF film capacitors (63 V box or polyester) | [`100nF 63V film capacitor`](https://www.aliexpress.com/w/wholesale-100nF-63V-film-capacitor.html?SortType=total_tranpro_desc) | 10 | B | Input DC block |
| TL072CP (DIP-8) | [`TL072CP DIP8`](https://www.aliexpress.com/w/wholesale-TL072CP-DIP8.html?SortType=total_tranpro_desc) | 10 | B | Fakes exist. LCSC sells real ones |
| DIP-8 IC sockets | [`DIP8 IC socket`](https://www.aliexpress.com/w/wholesale-DIP8-IC-socket.html?SortType=total_tranpro_desc) | 10 | B, W | |
| 1N5817 Schottky diodes | [`1N5817`](https://www.aliexpress.com/w/wholesale-1N5817.html?SortType=total_tranpro_desc) | 20 | B | Reverse-polarity protection |
| AMS1117-5.0 module | [`AMS1117 5V module`](https://www.aliexpress.com/w/wholesale-AMS1117-5V-module.html?SortType=total_tranpro_desc) | 3 | B | Gets warm in the pedal. An `L7805 TO220` with a small heatsink is the alternative |
| LEDs 3 mm, assorted colors | [`3mm LED kit`](https://www.aliexpress.com/w/wholesale-3mm-LED-kit.html?SortType=total_tranpro_desc) | 1 kit (100+) | all | |
| LED bezels 3 mm, chrome | [`3mm LED holder chrome`](https://www.aliexpress.com/w/wholesale-3mm-LED-holder-chrome.html?SortType=total_tranpro_desc) | 6 | B | |
| Tactile buttons 6x6 mm | [`tactile switch 6x6 kit`](https://www.aliexpress.com/w/wholesale-tactile-switch-6x6-kit.html?SortType=total_tranpro_desc) | 1 kit | B | Footswitches on the breadboard |

## 4. Modules

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| ESP32-S3 DevKitC-1 N16R8 (16 MB flash, 8 MB PSRAM) | [`ESP32-S3 DevKitC-1 N16R8`](https://www.aliexpress.com/w/wholesale-ESP32-S3-DevKitC-1-N16R8.html?SortType=total_tranpro_desc) | 2 | B, W | 1 + 1 spare. Step 1 receiver, then the pedal. Check "N16R8" |
| PCM1808 ADC module | [`PCM1808 module`](https://www.aliexpress.com/w/wholesale-PCM1808-module.html?SortType=total_tranpro_desc) | 3 | B | 1 + 2 spares |
| PCM5102A DAC module (GY-PCM5102) | [`PCM5102A I2S DAC module`](https://www.aliexpress.com/w/wholesale-PCM5102A-I2S-DAC-module.html?SortType=total_tranpro_desc) | 3 | B | 1 + 2 spares |

## 5. Front panel and enclosure hardware

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| B10K linear pot, 16 mm, solder lugs | [`B10K potentiometer 16mm`](https://www.aliexpress.com/w/wholesale-B10K-potentiometer-16mm.html?SortType=total_tranpro_desc) | 10 | B | 6 + spares. Note the shaft type (knurled 18T or D) |
| A10K log pot, 16 mm | [`A10K potentiometer 16mm`](https://www.aliexpress.com/w/wholesale-A10K-potentiometer-16mm.html?SortType=total_tranpro_desc) | 3 | B | Master volume (RV8) + spares |
| B10K push/pull pot | [`B10K push pull potentiometer`](https://www.aliexpress.com/w/wholesale-B10K-push-pull-potentiometer.html?SortType=total_tranpro_desc) | 2 | B | Katana VOLUME + 1 spare. Often a 24 mm body |
| Knobs for pedals | [`guitar pedal knob`](https://www.aliexpress.com/w/wholesale-guitar-pedal-knob.html?SortType=total_tranpro_desc) | 12 | B | 8 pots + spares. Must match the pot shaft. One easy to pull, for RV7 |
| Soft-touch momentary footswitch | [`momentary soft touch footswitch`](https://www.aliexpress.com/w/wholesale-momentary-soft-touch-footswitch.html?SortType=total_tranpro_desc) | 6 | B | 3 + 2 spares + 1 for later |
| 6.35 mm mono jack, open frame | [`6.35mm mono jack pedal`](https://www.aliexpress.com/w/wholesale-6.35mm-mono-jack-pedal.html?SortType=total_tranpro_desc) | 6 | B | 2 used |
| 6.35 mm stereo jack, open frame | [`6.35mm stereo jack pedal`](https://www.aliexpress.com/w/wholesale-6.35mm-stereo-jack-pedal.html?SortType=total_tranpro_desc) | 2 | B opt | Expression or headphones later |
| 2.1 mm DC jack, isolated, panel mount | [`2.1mm DC jack pedal`](https://www.aliexpress.com/w/wholesale-2.1mm-DC-jack-pedal.html?SortType=total_tranpro_desc) | 3 | B | Isolated = plastic body, for metal boxes |
| 1590XX aluminum enclosure | [`1590XX enclosure`](https://www.aliexpress.com/w/wholesale-1590XX-enclosure.html?SortType=total_tranpro_desc) | 1 | B | Buy after the breadboard works. 125B also fits |

## 6. Batteries, power and cables

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| 9 V isolated pedal power supply, center-negative | [`isolated pedal power supply`](https://www.aliexpress.com/w/wholesale-isolated-pedal-power-supply.html?SortType=total_tranpro_desc) | 1 | A, B | Known brand. Or a 9 V 1 A adapter + daisy chain |
| Daisy-chain cable, 2.1 mm, 5 plugs | [`pedal daisy chain cable`](https://www.aliexpress.com/w/wholesale-pedal-daisy-chain-cable.html?SortType=total_tranpro_desc) | 1 | A, B | Only with a non-isolated supply |
| Patch cables, 15 cm, right-angle, 6.35 mm | [`guitar patch cable 15cm right angle`](https://www.aliexpress.com/w/wholesale-guitar-patch-cable-15cm-right-angle.html?SortType=total_tranpro_desc) | 5 | A, B | Pedal chain |
| Guitar cable, 3 m | [`guitar cable 3m`](https://www.aliexpress.com/w/wholesale-guitar-cable-3m.html?SortType=total_tranpro_desc) | 1 | all | Skip if you have one |
| Adapters 6.35 mm female to 3.5 mm male, and the reverse | [`6.35mm to 3.5mm adapter`](https://www.aliexpress.com/w/wholesale-6.35mm-to-3.5mm-adapter.html?SortType=total_tranpro_desc) | 2 | all | Testing into headphones, phone or PC |
| USB audio interface, 2 inputs | [`USB audio interface`](https://www.aliexpress.com/w/wholesale-USB-audio-interface.html?SortType=total_tranpro_desc) | 1 | opt | Recording your dry guitar, reamping, measuring wireless latency. A known brand from a local shop is safer |

## 7. Optional extras for later (cheap now)

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| SSD1306 0.96" OLED, I2C | [`SSD1306 0.96 OLED I2C`](https://www.aliexpress.com/w/wholesale-SSD1306-0.96-OLED-I2C.html?SortType=total_tranpro_desc) | 2 | B opt | Settings and preset names |
| EC11 rotary encoder with push button | [`EC11 rotary encoder`](https://www.aliexpress.com/w/wholesale-EC11-rotary-encoder.html?SortType=total_tranpro_desc) | 2 | B opt | Menu |

## 8. Wireless guitar system

Design: [wireless-guitar-system/readme.md](wireless-guitar-system/readme.md). Transmitter: XIAO ESP32-C3 + nRF24L01+ + WM8960, LiPo, 3D-printed case. Receiver: the ESP32-S3 from section 4 + nRF24L01+ + WM8960, USB powered. Capacitors and resistors come from the kits in section 3.

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| Seeed XIAO ESP32-C3 | [`Seeed XIAO ESP32C3`](https://www.aliexpress.com/w/wholesale-Seeed-XIAO-ESP32C3.html?SortType=total_tranpro_desc) | 2 | W | Transmitter. 1 + 1 spare. Seeed Studio official store, not the C3 SuperMini |
| nRF24L01+ module, PCB antenna | [`nRF24L01+ module`](https://www.aliexpress.com/w/wholesale-nRF24L01%2B-module.html?SortType=total_tranpro_desc) | 4 | W, B | Breadboard tests, receiver, pedal. Many are Si24R1 copies: buy from stores with many reviews |
| nRF24L01+ mini SMD module | [`nRF24L01+ SMD mini`](https://www.aliexpress.com/w/wholesale-nRF24L01%2B-SMD-mini.html?SortType=total_tranpro_desc) | 3 | W | Transmitter case. 1 + 2 spares |
| nRF24 adapter board with 3.3 V regulator | [`nRF24L01 adapter 3.3V`](https://www.aliexpress.com/w/wholesale-nRF24L01-adapter-3.3V.html?SortType=total_tranpro_desc) | 2 | W, B | Breadboard: the nRF24 pins do not fit a breadboard |
| WM8960 codec module | [`WM8960 audio module`](https://www.aliexpress.com/w/wholesale-WM8960-audio-module.html?SortType=total_tranpro_desc) | 3 | W | Transmitter + receiver + spare. Pick the smallest board, 3.3 V, not the Raspberry Pi HAT |
| MCP6002 op-amp, DIP-8 | [`MCP6002 DIP8`](https://www.aliexpress.com/w/wholesale-MCP6002-DIP8.html?SortType=total_tranpro_desc) | 5 | W | Transmitter buffer on the breadboard. Runs on 3.3 V |
| MCP6002 op-amp, SOIC-8 | [`MCP6002 SOIC8`](https://www.aliexpress.com/w/wholesale-MCP6002-SOIC8.html?SortType=total_tranpro_desc) | 5 | W | Only for a custom PCB |
| LiPo 1S 3.7 V, 400-600 mAh, with protection board | [`602535 lipo 500mAh`](https://www.aliexpress.com/w/wholesale-602535-lipo-500mAh.html?SortType=total_tranpro_desc) | 2 | W | Transmitter. 1 + 1 spare. Choose the size after the case is drawn |
| JST-PH 2.0 2-pin pigtails, male + female | [`JST PH 2.0 2pin pigtail`](https://www.aliexpress.com/w/wholesale-JST-PH-2.0-2pin-pigtail.html?SortType=total_tranpro_desc) | 1 | W | Battery can be unplugged |
| Mini slide switch SPDT | [`SS12D00 slide switch`](https://www.aliexpress.com/w/wholesale-SS12D00-slide-switch.html?SortType=total_tranpro_desc) | 10 | W | Transmitter power switch |
| 6.35 mm mono male plug, metal, solder type | [`6.35mm mono plug metal solder`](https://www.aliexpress.com/w/wholesale-6.35mm-mono-plug-metal-solder.html?SortType=total_tranpro_desc) | 3 | W | Transmitter. Straight |
| 6.35 mm mono male plug, right-angle | [`6.35mm right angle plug solder`](https://www.aliexpress.com/w/wholesale-6.35mm-right-angle-plug-solder.html?SortType=total_tranpro_desc) | 2 | W | If a straight case does not fit your guitar's jack |
| 3.5 mm stereo jack, panel mount | [`3.5mm stereo jack panel mount`](https://www.aliexpress.com/w/wholesale-3.5mm-stereo-jack-panel-mount.html?SortType=total_tranpro_desc) | 3 | W | Receiver P3 output. The 6.35 mm jack comes from section 5 |
| Solderable breadboard, half size | [`solderable breadboard half`](https://www.aliexpress.com/w/wholesale-solderable-breadboard-half.html?SortType=total_tranpro_desc) | 4 | W, B | Same layout as a breadboard, so the circuit moves over 1:1 |
| Double-sided prototype board, 3 x 7 cm | [`double sided PCB prototype 3x7`](https://www.aliexpress.com/w/wholesale-double-sided-PCB-prototype-3x7.html?SortType=total_tranpro_desc) | 10 | W | Cut to fit the transmitter case |
| M2 heat-set inserts + M2 screws kit | [`M2 heat set insert kit`](https://www.aliexpress.com/w/wholesale-M2-heat-set-insert-kit.html?SortType=total_tranpro_desc) | 1 | W | Closes the 3D-printed case |

Custom PCB for the transmitter (optional): JLCPCB or PCBWay, about 5 USD for 5 boards plus shipping.

## 9. Wireless alternative: nRF52840 (longer battery life)

Only if you choose the [nRF52840 alternative](wireless-guitar-system/readme.md#9-alternative-nrf52840-for-a-longer-battery-life). It replaces the XIAO ESP32-C3 and the nRF24 modules.

| Item | AliExpress link (sorted by orders) | Buy | For | Notes |
|---|---|---|---|---|
| Seeed XIAO nRF52840 (not the Sense) | [`Seeed XIAO nRF52840`](https://www.aliexpress.com/w/wholesale-Seeed-XIAO-nRF52840.html?SortType=total_tranpro_desc) | 3 | W opt | 2 + 1 spare. Seeed Studio official store |
| CMSIS-DAP / DAPLink debug probe (SWD) | [`DAPLink CMSIS-DAP`](https://www.aliexpress.com/w/wholesale-DAPLink-CMSIS-DAP.html?SortType=total_tranpro_desc) | 1 | W opt | Only to recover a board if its bootloader is erased |

## Cost per item

Target prices in EUR, including Danish VAT (25%), for the quantities in the sections above. Only the XIAO, WM8960 and PCM5102A prices were seen on real listings; the rest are typical AliExpress prices. Pay up to about 20% more than the target for a better-reviewed listing; much less than the target is often a fake or a coupon price.

### 1. Tools

| Item | Qty | Target each | Target total |
|---|---|---|---|
| Multimeter | 1 | 20 | 20 |
| Soldering iron (Pinecil V2) | 1 | 35 | 35 |
| 65 W USB-C PD charger for the Pinecil, if you have none | 1 | 17 | 17 |
| Spare iron tip | 1 | 5 | 5 |
| Solder 0.8 mm, 100 g | 1 | 8 | 8 |
| Flux | 1 | 3 | 3 |
| Brass tip cleaner | 1 | 3 | 3 |
| Desoldering pump | 1 | 3 | 3 |
| Desoldering wick 2 mm | 2 | 2 | 4 |
| Flush side cutters | 1 | 4 | 4 |
| Wire stripper | 1 | 6 | 6 |
| Tweezers set | 1 | 4 | 4 |
| PCB holder (helping hands) | 1 | 10 | 10 |
| Step drill bit set | 1 | 10 | 10 |
| Automatic center punch | 1 | 4 | 4 |
| Small nut driver / socket set | 1 | 8 | 8 |
| Safety glasses | 1 | 3 | 3 |
| Alligator clip test leads (pack of 10) | 1 | 4 | 4 |
| USB-C data cable | 3 | 2.5 | 7.5 |
| **Subtotal** | | | **159** |

### 2. Wire and prototyping

| Item | Qty | Target each | Target total |
|---|---|---|---|
| Solid-core wire 22 AWG kit | 1 | 7 | 7 |
| Stranded silicone wire 24 AWG kit | 1 | 6 | 6 |
| Heat-shrink tubing set | 1 | 3 | 3 |
| Breadboard, 830 points | 4 | 1.5 | 6 |
| Jumper wires (M-M, M-F, F-F packs) | 3 | 1.5 | 4.5 |
| Male pin headers, 40 pin | 10 | 0.15 | 1.5 |
| Female pin headers, 40 pin | 10 | 0.25 | 2.5 |
| Perfboard / stripboard | 5 | 0.6 | 3 |
| DC jack to screw terminal adapter | 2 | 0.6 | 1.2 |
| M3 nylon standoff kit | 1 | 5 | 5 |
| Double-sided foam tape | 1 | 2 | 2 |
| **Subtotal** | | | **42** |

### 3. Small components

| Item | Qty | Target each | Target total |
|---|---|---|---|
| Resistor kit, 600+ pcs | 1 | 5 | 5 |
| Ceramic capacitor kit | 1 | 3 | 3 |
| Extra 100 nF ceramic capacitors (pack of 50) | 1 | 1.5 | 1.5 |
| Electrolytic capacitor kit | 1 | 4 | 4 |
| 100 nF film capacitors | 10 | 0.15 | 1.5 |
| TL072CP | 10 | 0.3 | 3 |
| DIP-8 IC sockets | 10 | 0.1 | 1 |
| 1N5817 diodes (pack of 20) | 1 | 1 | 1 |
| AMS1117-5.0 module | 3 | 0.7 | 2.1 |
| LED kit 3 mm | 1 | 2 | 2 |
| LED bezels 3 mm | 6 | 0.35 | 2.1 |
| Tactile button kit | 1 | 2 | 2 |
| **Subtotal** | | | **28** |

### 4. Modules

| Item | Qty | Target each | Target total |
|---|---|---|---|
| ESP32-S3 DevKitC-1 N16R8 | 2 | 9 | 18 |
| PCM1808 module | 3 | 3 | 9 |
| PCM5102A module | 3 | 1.8 | 5.4 |
| **Subtotal** | | | **32** |

### 5. Front panel and enclosure hardware

| Item | Qty | Target each | Target total |
|---|---|---|---|
| B10K pot, 16 mm | 10 | 0.4 | 4 |
| A10K pot, 16 mm | 3 | 0.5 | 1.5 |
| B10K push/pull pot | 2 | 2.5 | 5 |
| Knobs | 12 | 0.5 | 6 |
| Soft-touch momentary footswitch | 6 | 1.5 | 9 |
| 6.35 mm mono jack | 6 | 0.6 | 3.6 |
| 2.1 mm DC jack, isolated | 3 | 0.5 | 1.5 |
| 1590XX enclosure | 1 | 12 | 12 |
| 6.35 mm stereo jack (optional) | 2 | 0.8 | 1.6 |
| **Subtotal** | | | **43** (+2 optional) |

### 6. Batteries, power and cables

| Item | Qty | Target each | Target total |
|---|---|---|---|
| 9 V isolated pedal power supply | 1 | 35 | 35 |
| Patch cables, 15 cm | 5 | 1.6 | 8 |
| Guitar cable, 3 m | 1 | 6 | 6 |
| Adapters 6.35 / 3.5 mm | 2 | 1 | 2 |
| Daisy-chain cable (only with a non-isolated supply) (optional) | 1 | 4 | 4 |
| USB audio interface (optional) | 1 | 45 | 45 |
| **Subtotal** | | | **51** (+49 optional) |

### 7. Optional extras for later

| Item | Qty | Target each | Target total |
|---|---|---|---|
| SSD1306 OLED (optional) | 2 | 2.5 | 5 |
| EC11 rotary encoder (optional) | 2 | 0.8 | 1.6 |
| **Subtotal (optional)** | | | **7** |

### 8. Wireless guitar system

| Item | Qty | Target each | Target total |
|---|---|---|---|
| Seeed XIAO ESP32-C3 | 2 | 7 | 14 |
| nRF24L01+ module, PCB antenna | 4 | 1.2 | 4.8 |
| nRF24L01+ mini SMD module | 3 | 1.5 | 4.5 |
| nRF24 adapter board with 3.3 V regulator | 2 | 0.8 | 1.6 |
| WM8960 codec module | 3 | 10 | 30 |
| MCP6002 op-amp, DIP-8 | 5 | 0.5 | 2.5 |
| MCP6002 op-amp, SOIC-8 | 5 | 0.4 | 2 |
| LiPo 1S 3.7 V, 400-600 mAh, with protection board | 2 | 3.5 | 7 |
| JST-PH 2.0 2-pin pigtails, male + female | 1 | 2 | 2 |
| Mini slide switch SPDT | 10 | 0.15 | 1.5 |
| 6.35 mm mono male plug, metal, solder type | 3 | 2 | 6 |
| 6.35 mm mono male plug, right-angle | 2 | 2 | 4 |
| 3.5 mm stereo jack, panel mount | 3 | 0.4 | 1.2 |
| Solderable breadboard, half size | 4 | 1.5 | 6 |
| Double-sided prototype board, 3 x 7 cm | 10 | 0.3 | 3 |
| M2 heat-set inserts + M2 screws kit | 1 | 5 | 5 |
| **Subtotal** | | | **95** |

### 9. Wireless alternative: nRF52840 (replaces the XIAO ESP32-C3 and nRF24 rows)

| Item | Qty | Target each | Target total |
|---|---|---|---|
| Seeed XIAO nRF52840 (optional) | 3 | 13 | 39 |
| DAPLink debug probe (optional) | 1 | 5 | 5 |
| **Subtotal (optional)** | | | **44** |

## Summary

| | Target |
|---|---|
| Digital pedal (sections 2-6) | 196 |
| Wireless system, with spares (section 8) | 95 |
| **Everything required** | **291** |
| Optional items (marked above) | up to 57 |
| Wireless alternative nRF52840 (section 9) | +19 if it replaces the ESP32-C3 and nRF24 parts |
| Analog pedals (postponed, not included) | see [analog-pedals/shopping-list.md](analog-pedals/shopping-list.md) |
| **Tools (section 1)** | **159** |

The wireless system alone, one transmitter and one receiver without spares, costs about 50 EUR. The ESP32-S3 is counted in section 4 because the pedal reuses it. 3D-printing filament not counted.

### Auxiliary equipment (not in the sections above)

| Item | Estimate | Needed? |
|---|---|---|
| Drill, if you have none (local shop) | 30-50 | For the enclosures |
| Isopropyl alcohol (local shop) | 5 | Cleaning flux off boards |
| Silicone soldering mat | 5-8 | Recommended |
| Small fume extractor fan | 15-25 | Recommended for indoor soldering |
| PETG filament, 1 kg | 15-20 | For the wireless cases, if you have a 3D printer |
| Headphones or small powered speaker | 0-30 | Probably already at home |
| **Auxiliary total** | **about 5 (drill at home) or 35-55 (no drill); up to about 140 with everything** | |

## Saving on the wireless part

| Change | Saves |
|---|---|
| Receiver with a spare PCM5102A instead of a WM8960 (no headphone amp on the 3.5 mm jack) | about 10 |
| Skip the spare XIAO and WM8960 | about 17 |

## Check before you order

- [ ] Pot shafts and knobs are the same type (knurled 18T or D-shaft, 6 mm).
- [ ] The ESP32-S3 board is the DevKitC-1 N16R8 (8 MB PSRAM).
- [ ] The resistor kit contains 1 MOhm.
- [ ] The power supply is 9 V, center-negative.
