# Shot Timer MK1 — Wiring and Protoboard Layout

> **DRAFT** transcribed from the hand-drawn build notes (notebook pages, Oct 2026).
> Items marked ❓ need to be confirmed against a real unit before this is final.

## Board

- **ElectroCookie** solderable protoboard, rows 1–30.
- Each row is two 5-hole strips: **A–E** (left) and **F–J** (right), split by the center gap.
- Power rails on both edges: **left rail = 9 V (switched) / GND**, **right rail = 5 V / GND**. ❓
- Wire leads exit about 4 cm from the board edge; buzzer leads about 9 cm. ❓

## Arduino Nano placement

The Nano straddles the center gap in **columns C and G, rows 16–30**, with VIN at G16.
Every other hole in the same strip as a pin is connected to that pin.

| Row | Column C (A–E strip) | Column G (F–J strip) |
|---|---|---|
| 16 | TX1 | **VIN** |
| 17 | RX0 | **GND** |
| 18 | RST | RST |
| 19 | **GND** | **5V** |
| 20 | D2 | A7 |
| 21 | **D3** (buzzer) | A6 |
| 22 | D4 | **A5** (SCL) |
| 23 | D5 | **A4** (SDA) |
| 24 | D6 | A3 |
| 25 | D7 | A2 |
| 26 | **D8** (STOP) | **A1** (battery divider) |
| 27 | **D9** (START) | **A0** (mic) |
| 28 | D10 | AREF |
| 29 | D11 | 3V3 |
| 30 | D12 | **D13** (white LED) |

## Connections

### Power
| From | To | Notes |
|---|---|---|
| Battery + (clip red) | SW1 input | |
| SW1 output (9 V switched) | Row 1 A–E (divider top) and INA219 **Vin+** | ❓ exact board holes |
| INA219 **Vin−** | Nano **VIN** (row 16 F–J, blue wire at I16) | Added 10/16/2026; no direct SW1→VIN wire may remain |
| Battery − (clip black) | GND | |
| Nano 5V (H19) | 5 V rail | |
| Nano GND (H17) | GND rail | |

### Battery voltage divider (fallback sense on A1)
| Part | Holes | Net |
|---|---|---|
| R1 10k | C1 → G1 | 9 V switched (row 1 A–E) → A1 node (row 1 F–J) |
| R2 6.9k (2.2k + 4.7k in series) | I1 → B2 | A1 node → GND (row 2 A–E) |
| Jumper | J1 → H26 | A1 node → Nano A1 |

### I2C devices
| Device | Pin | Connects to |
|---|---|---|
| OLED (header in I5–I8, mounted on the **underside**) | SDA (I5) | J5 → J23 (A4) |
| | SCL (I6) | J6 → J22 (A5) |
| | VCC (I7) | J7 → 5 V |
| | GND (I8) | J8 → GND |
| INA219 | SDA (yellow) | I23 (A4) |
| | SCL (white) | I22 (A5) |
| | Vcc | 5 V (never 9 V) |
| | GND | GND |

### Microphone circuit
| Part | Holes | Net |
|---|---|---|
| TRRS breakout R2 (mic signal) | I12 | Mic node (row 12 F–J) |
| TRRS breakout Sleeve | GND | |
| R5 2.2k | J14 → J12 | 5 V (row 14 F–J, H14) → mic node |
| C1 10 µF | + H12, − E14 | Mic node → bias node (row 14 A–E) |
| R3 100k | I14 → B14 | 5 V → bias node |
| R4 90k (22k + 68k in series) | C14 → I13 | Bias node → row 13 F–J (GND ❓) |
| Jumper | G12 → I27 | Mic node → Nano A0 |

❓ On the board, A0 and the capacitor's + side are both on the mic node (row 12). On the
KiCad sheet the 100k/90k divider is drawn biasing the mic side. Confirm which matches the build.

### Buttons, buzzer and LED
| Part | Signal wire | Ground wire |
|---|---|---|
| SW3 STOP (red 16 mm) | B26 → D8 | GND |
| SW2 START (green 16 mm) | B27 → D9 | GND |
| LS1 buzzer | red, B21 → D3 | black, GND |
| D1 white LED (panel mount) | green, I30 → D13 | black, GND |

❓ No series resistor for the LED appears on the board. Confirm the panel-mount LED has a
built-in resistor (the user manual currently says "D13 through 1k").

## Free Nano pins
D2, D4–D7, D10–D12, A2, A3, A6, A7.
