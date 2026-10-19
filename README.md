# Shot Timer MK1

An open-source shot timer for live-fire and dry-fire practice, built on an Arduino Nano.
Press START, wait for the random beep, shoot, and press STOP to see your draw time, splits,
totals and average on a two-color OLED.

Designed and built by **Huckleberry Innovations** — Lexington, SC.
Want one ready to go? Order at [huckleberryinnovations.com](https://huckleberryinnovations.com)
or email order@huckleberryinnovations.com.

## Features

- Random 3–8 s start delay and 0.5 s start beep
- Up to 60 shots per string, shown in seconds with two decimals
- Result pages: 1ST SHOT (draw), AVERAGE, SPLITS, TOTALS, BATTERY
- Last string saved through power-off
- Adjustable sound threshold (0–500, default 150) plus a live mic test mode
- Battery gauge (INA219 coulomb counting) with percentage on every screen,
  LOW and CHARGE warnings, and automatic capacity learning
- About 16 hours per charge on a rechargeable 9 V USB-C lithium pack

## Repository layout

```
shot-timer-mk1/
├── firmware/ShotTimerMk1_v1_7_5/   Arduino sketch (current: v1.7.5)
├── hardware/
│   ├── BOM.csv                     Parts list
│   ├── WIRING.md                   Wiring and protoboard layout
│   ├── schematic/                  KiCad project + PDF export
│   └── enclosure/                  STL / 3MF print files, USB port plug, drawing package
└── docs/                           Project Book, user manual, quick-start card, hardware build guide
```

| Folder | License |
|---|---|
| [`firmware/`](firmware) | GPL-3.0-or-later |
| [`hardware/`](hardware) | CERN-OHL-S-2.0 |
| [`docs/`](docs) | CC BY-SA 4.0 |

## Hardware

| Part | Notes |
|---|---|
| Arduino Nano (ATmega328P) | Main controller |
| 0.96" SSD1306 128×64 OLED, yellow/blue | I2C 0x3C |
| INA219 current sensor, 0.1 Ω shunt | I2C 0x40, high-side in the battery line |
| Clip-on lavalier mic, 3.5 mm TRRS jack | Mic input on A0 |
| Buzzer | D3 |
| START / STOP buttons | D9 / D8 to ground (internal pull-ups) |
| White status LED (panel mount) | D13 |
| Latching lit power switch | In the battery + line |
| 9 V USB-C rechargeable lithium battery | Use rechargeable lithium only |

Full quantities are in the [parts list (BOM.csv)](hardware/BOM.csv).

## Building one

| Document | What it covers |
|---|---|
| [Hardware Build Guide](docs/Shot_Timer_Mk1_Hardware_Build_Guide.pdf) | Parts and tools, protoboard layout diagram, step-by-step soldering with photos, checkout test, final assembly, packaging |
| [Wiring and board layout](hardware/WIRING.md) | Every connection, by circuit and protoboard hole |
| [Schematic (PDF)](hardware/schematic/shot-timer-mk1_schematic.pdf) | Circuit diagram; open [`hardware/schematic/`](hardware/schematic) in KiCad 7 or newer to edit |
| [Enclosure drawings](hardware/enclosure/Shot_Timer_Mk1_Enclosure_Drawings.pdf) | Renders, standard views with dimensions, exploded view, part details |
| [Enclosure print files](hardware/enclosure) | STL bodies for each color and multi-color 3MF projects |
| [USB port plug](hardware/enclosure/port-plug-measured.stl) | Optional press-fit plug for the USB programming port slot |
| [User manual, Part 2](docs/Shot_Timer_Mk1_Manual_v1.7.5.pdf) | How the firmware, battery gauge and hardware work |

The enclosure is two PLA+ parts (bottom shell and lid), each printed in several colors.
All STL files share one coordinate system, so loading them together in a slicer puts every
color body in place.

### USB port plug (optional)

[`port-plug-measured.stl`](hardware/enclosure/port-plug-measured.stl) is a small press-fit plug
that covers the 12 × 7 mm USB programming port slot in the end of the bottom shell, keeping
debris and water out. A grip tab under the flange lets you pull it with a fingernail when you
need to flash the firmware. It is sized from a measured slot (11.92 × 6.88 mm) with small crush
ribs for a snug fit in PLA. Print it flange-down; no supports are needed.

> The hardware documentation is a first release. A few wiring details are still being
> confirmed on a built unit; they are marked ❓ in the build guide and WIRING.md.

## Flashing the firmware

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In **Library Manager**, install **Adafruit SSD1306** and **Adafruit GFX Library**.
   (Wire and EEPROM are built in; no INA219 library is needed.)
3. Open `firmware/ShotTimerMk1_v1_7_5/ShotTimerMk1_v1_7_5.ino`.
4. Select **Tools → Board → Arduino Nano**. Clone boards may need
   **Processor → ATmega328P (Old Bootloader)**.
5. Connect with a data USB cable (not a charge-only cable) and click **Upload**.

Updating keeps the saved threshold, last string and learned battery capacity.

## Project Book

The [Project Book](docs/Shot%20Timer%20MK1%20Project%20Book.pdf) puts everything about the
Shot Timer MK1 in one document: how it works, hardware and power, firmware and version history,
the v6 enclosure, the build, a map of all the other documents, design decisions, and the list
of open items still being confirmed.

## Using it

See the [quick-start card](docs/Shot_Timer_MK1_Quick_Start_Fold_v1.7.5.pdf) for the basics and the
[full manual](docs/Shot_Timer_Mk1_Manual_v1.7.5.pdf) for everything else.

## License

This project is open source:

- **Firmware** — [GNU GPL v3.0 or later](LICENSE)
- **Hardware design** — [CERN Open Hardware Licence v2, Strongly Reciprocal](hardware/LICENSE)
- **Documentation** — [Creative Commons Attribution-ShareAlike 4.0](docs/LICENSE)

You are free to build, modify and sell your own units. If you distribute a modified
version, you must share your changes under the same license and credit the original project.

© 2026 Josue F. Kure / Huckleberry Innovations LLC.
The names "Huckleberry Innovations" and "Shot Timer MK1" are not covered by these licenses;
please use a different name for your own builds and derivatives.
