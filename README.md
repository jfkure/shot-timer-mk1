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

| Folder | Contents | License |
|---|---|---|
| [`firmware/`](firmware) | Arduino sketch (current: v1.7.5) | GPL-3.0-or-later |
| [`hardware/`](hardware) | Schematic and enclosure files | CERN-OHL-S-2.0 |
| [`docs/`](docs) | Full user manual and fold-out quick-start card (PDF) | CC BY-SA 4.0 |

## Hardware

| Part | Notes |
|---|---|
| Arduino Nano (ATmega328P) | Main controller |
| 0.96" SSD1306 128×64 OLED, yellow/blue | I2C 0x3C |
| INA219 current sensor, 0.1 Ω shunt | I2C 0x40, high-side in the battery line |
| Clip-on lavalier mic, 3.5 mm TRRS jack | Mic input on A0 |
| Buzzer | D3 |
| START / STOP buttons | D9 / D8 to ground (internal pull-ups) |
| White status LED | D13 through 1 kΩ |
| Latching lit power switch | In the battery + line |
| 9 V USB-C rechargeable lithium battery | Use rechargeable lithium only |

Wiring details, the battery gauge design and assembly notes are in Part 2 of the
[user manual](docs/Shot_Timer_Mk1_Manual_v1.7.5.pdf).

## Flashing the firmware

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In **Library Manager**, install **Adafruit SSD1306** and **Adafruit GFX Library**.
   (Wire and EEPROM are built in; no INA219 library is needed.)
3. Open `firmware/ShotTimerMk1_v1_7_5/ShotTimerMk1_v1_7_5.ino`.
4. Select **Tools → Board → Arduino Nano**. Clone boards may need
   **Processor → ATmega328P (Old Bootloader)**.
5. Connect with a data USB cable (not a charge-only cable) and click **Upload**.

Updating keeps the saved threshold, last string and learned battery capacity.

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
