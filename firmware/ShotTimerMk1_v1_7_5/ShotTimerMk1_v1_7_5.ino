/*
  Shot Timer Mk1 firmware
  Copyright (C) 2026 Josue F. Kure / Huckleberry Innovations LLC
  https://github.com/jfkure/shot-timer-mk1

  SPDX-License-Identifier: GPL-3.0-or-later

  This program is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
  details: https://www.gnu.org/licenses/gpl-3.0.html

  "Huckleberry Innovations" and "Shot Timer MK1" are names of Huckleberry
  Innovations LLC and are not licensed under the GPL.
*/

/*
  Shot Timer Mk1 v1.7.5
  Target: Arduino Nano (ATmega328P).

  v1.7.5 - Display refinements (software only; data carries over):
    * Long gaps: a gap of more than 65.5 s between two shots (or from the beep
      to the first shot) is measured with the 32-bit clock and stored as exactly
      65.53 s, shown as "65.53+" - a minimum - instead of wrapping to a wrong
      small number. TOTALS and AVERAGE that include such a gap also show "+".
      List rows drop the space, then the '#', if needed to fit 10 characters.
    * CHARGE screen shows the percentage: CHARGE flashes at size 2 with a steady
      "4%" beside it, matching the LOW screen. Divider units are unchanged.

  v1.7.4 - Gauge accuracy across power cycles (software only; data carries over):
    * Charge used is saved every 1 mAh (was 2). EEPROM wear: ~33 writes per slot
      per full discharge across the 16-slot ring, about 3,000 full charges of
      life - longer than the battery's.
    * At power-up, half a save interval (0.5 mAh) is added back to the count to
      offset the average amount lost at the last switch-off. Previously each
      power cycle could lose up to 2 mAh, always making the gauge read high.
      Now each cycle is within about +/-0.5 mAh and errors don't build up.
    * The count is also saved at the end of every string.
    * Test mode fix: leaving test mode could start a string (random wait, GO!,
      beep). The START hold that opened test mode was never seen released, so
      on exit it was mistaken for the release of a tap. The hold is now marked
      as used, and the START state is cleared on exit.
    * MARK FULL? fix: releasing START after the 3 s hold could bounce and cancel
      the prompt instantly. START now has to stay released 150 ms before a START
      tap (held 40 ms or more) can cancel, and the 5 s timeout starts on release.

  v1.7.3 - Back-pocket list (software only; same wiring, EEPROM carries over):
    * Long strings: TOTALS and AVERAGE are now built from a 32-bit sum of the
      16-bit splits, so they stay correct for strings of any length. The only
      limit left is a single gap between two shots must be under 65.5 s.
      Big-number and list layouts handle 3-digit seconds (e.g. 104.37s).
    * Battery gauge in the corner: small icon + % in the top-right of the yellow
      strip on 1ST SHOT, AVERAGE, SPLITS and TOTALS, and inline on the start
      screen. Redrawn when the % changes. To make room: AVG SPLIT is titled
      AVERAGE, the LAST tag moved to the bottom as LAST STRING, and the page
      number sits right after the SPLITS/TOTALS title. INA219 units only.
    * FULL confirm: holding START 3 s on the BATTERY page now shows MARK FULL?
      Tap STOP = yes, tap START = no, or wait 5 s to cancel.
    * Quieter chirps: off-page LOW/CHARGE chirps pause for 3 minutes after any
      button press or the end of a string, so reviewing results stays quiet.
      The BATTERY page keeps its own beeps.
    * Removed the leftover 7-segment code; D4-D7 and D10-D12 are now unused
      inputs, free for future features.
    * RAM: USB serial logging is now off by default (DEBUG_SERIAL 0), freeing
      157 bytes of SRAM. Fixes stray pixels bottom-right of the screen while
      recording (stack running into the end of the OLED buffer). The split
      table on the serial port returns if DEBUG_SERIAL is set to 1.
    * Sensitivity menu exits on the first STOP tap (the exit tap used to reopen
      it). On result pages STOP now flips the page on release, so holding STOP
      to open the menu no longer flips a page first. STOP still ends a running
      string the instant it is pressed.
    * Test mode exits on the first STOP tap (STOP is watched continuously
      instead of once per ~60 ms screen refresh).
    * Threshold screen hint shortened to "START +10  hold:FAST" so it fits on
      one line; start screen shows the version on its own line with the same
      corner battery gauge as the result pages.
    * USB-only fix: "no battery" cutoff raised from 2.0 V to 6.0 V. Voltage
      back-fed through the Nano regulator on USB power no longer shows as
      CHARGE / 0 mA; the BATTERY page shows NO BATTERY and all alarms stay off.

  v1.7.2 - Gauge works for packs that hold 9 V until they shut off (software only):
    * Learning without a voltage cliff: when the pack is marked FULL, if the
      discharge that just ended was not already learned at the cliff and used
      >= 70% of the current capacity estimate, it is taken as a full drain and
      its saved count becomes the capacity. Short top-ups (< 70%) are ignored.
      First measurement replaces the default; later ones blend 3:1 toward the
      newest so the estimate follows battery aging. Cliff learning (packs that
      do sag) still works as before.
    * Percentage-based warnings: LOW at <= 15%, CHARGE at <= 5%. Combined with
      the voltage thresholds - whichever is worse wins - so packs that sag and
      packs that don't both get warned. If usage runs past the estimate, the
      gauge holds at 1% (CHARGE) until the next FULL mark corrects it.
    * Warnings on every page: while idle on any page other than BATTERY, LOW
      gives a single beep every 60 s and CHARGE a double beep every 30 s. Silent
      during the start delay, runs, test mode and the threshold menu. The
      BATTERY page keeps its faster cadence (10 s / 5 s).
    * LOW screen shows the percentage beside the flashing LOW; LOW and CHARGE
      fill bars match the gauge.

  v1.7.1 - Coulomb-counting battery gauge (software only; needs the INA219):
    * Current is sampled every 500 ms on every screen (idle, delay, running,
      menus) and integrated into "charge used" (micro-amp-hours). Each sample
      is ~0.3 ms of I2C, so shot detection is unaffected.
    * BATTERY page shows remaining charge as a big percentage with a matching
      fill bar (volts + mA still top-right). The voltage LOW / CHARGE warnings
      and beeps still override the display - they are the safety backstop.
    * After charging the battery: open the BATTERY page and HOLD START ~3 s.
      Shows "FULL" and resets the gauge to 100%. (On every other page, holding
      START still opens hidden test mode.) Until the first FULL reset the page
      shows "--%" with a reminder.
    * Self-calibrating capacity: starts at 500 mAh (USB-C 9 V packs usually
      print the internal 3.7 V cell's mAh, not the 9 V output). When a pack
      that was marked FULL reaches the CHARGE threshold (7.9 V, held 5 s), the
      charge used is saved as the real capacity - first time replaces the
      default, later drains are averaged in. So 0% lines up with CHARGE.
    * Charge used is saved to EEPROM every 2 mAh, rotated across 16 slots for
      wear leveling. Flipping SW1 off loses at most ~2 mAh of count.
    * Factory reset (both buttons) leaves the learned capacity alone - it
      belongs to the battery, not the user settings.
    * Hidden test screen shows charge used / capacity in mAh.
    * Divider-only units (no INA219) behave exactly like v1.7.0.

  v1.7.0 - INA219 battery sensing (hardware + software):
    * Battery voltage now comes from an INA219 current/voltage sensor on the
      shared I2C bus (address 0x40, A0/A1 pads open), wired HIGH-SIDE:
        Battery + -> SW1 -> INA219 Vin+ ; INA219 Vin- -> Nano VIN (+9V net)
        INA219 Vcc -> Nano 5V ; GND -> common GND ; SDA -> A4 ; SCL -> A5
      Reported voltage = bus voltage (Vin-) + shunt drop = true battery-side volts.
      No calibration offset needed (the divider needed +0.23 V).
    * BATTERY screen also shows live current draw (mA) under the voltage when
      the INA219 is present - a tuning/QC aid and the basis for future
      runtime/charge estimation.
    * Driver is direct register access (no Adafruit_INA219 library) to save
      flash and SRAM. INA219 configured: 16 V range, +/-40 mV shunt range
      (+/-400 mA on the 0.1 ohm R100 shunt), bus ADC 8-sample avg,
      shunt ADC 128-sample avg, continuous.
    * Automatic fallback: if no INA219 answers at boot (or a read fails), the
      legacy A1 divider (10k/6.8k) is used exactly as in v1.6.4. Same firmware
      runs on both hardware revisions. The hidden test screen shows which
      source is active ("BATT:INA219" or "BATT:DIVIDER").
    * Battery thresholds are now expressed in millivolts (BATT_MV_*), shared by
      both sources. Values unchanged: WARN 8.50 V, LOW (cliff) 7.90 V.

  v1.6.4 - LOW battery single beep every 10 s while the BATTERY page is shown.
  v1.6.3 - Two-stage battery warning (LOW ~8.5 V, CHARGE ~7.9 V) + cliff buzzer.
  v1.6.2 - 8-sample ADC averaging for battery readout.
  v1.6.1 - BATTERY page live refresh.
  v1.6.0 - EEPROM persistence (threshold + last string), cliff-style battery
           screen, factory reset (hold both buttons ~3 s), SRAM rework (no RAM
           mirror of saved splits; free RAM on hidden test screen).
  v1.5.x - Two-screen summary (1ST SHOT / AVG SPLIT), two-color OLED layout.

  Earlier features: random 3-8 s start delay, times in seconds, 400 kHz I2C,
  strings in flash (F()), integer-only math (no floating-point library), named
  state enum.

  Page order when reviewing (cycle with STOP):
    1ST SHOT -> AVG SPLIT -> SPLITS -> TOTALS -> BATTERY
*/

#include <Wire.h>                // I2C communication
#include <Adafruit_GFX.h>        // Graphics library for OLED
#include <Adafruit_SSD1306.h>    // OLED driver
#include <EEPROM.h>

// ================= Debug logging (v1.7.3) =================
// USB serial logging is OFF by default: the Serial object and its buffers cost
// 157 bytes of SRAM, and that stack headroom is needed while recording (too
// little lets the stack overwrite the end of the OLED buffer, showing stray
// pixels bottom-right). Set to 1 only for bench debugging.
#define DEBUG_SERIAL 0
#if DEBUG_SERIAL
  #define DBG_BEGIN()    Serial.begin(115200)
  #define DBG_PRINT(x)   Serial.print(x)
  #define DBG_PRINTLN(x) Serial.println(x)
#else
  #define DBG_BEGIN()
  #define DBG_PRINT(x)
  #define DBG_PRINTLN(x)
#endif              // Persistent threshold + last-string storage

// ================= OLED configuration =================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDR     0x3C       // I2C address for SSD1306

// Two-color panel bands (fixed in hardware)
#define YELLOW_BAND_H 16         // rows 0..15 render yellow
#define BLUE_TOP      16         // blue band starts here (rows 16..63)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= Battery monitoring (integer math, no floating point) =================
// Primary source: INA219 (I2C 0x40), high-side between SW1 and Nano VIN.
// Fallback source: legacy divider on A1 (R1 = 10k high side, R2 = 6.8k low side).
// A boost-regulated 9 V li-ion holds ~9 V flat then drops fast ("cliff"), so a
// percentage is misleading. Instead: OK above WARN, flashing LOW between WARN and
// LOW, flashing CHARGE + buzzer below LOW. Tune the BATT_MV_* values to your pack
// using the live X.XXV readout on the BATTERY screen.
const uint8_t  INA219_ADDR       = 0x40;
const uint8_t  INA219_REG_CONFIG = 0x00;
const uint8_t  INA219_REG_SHUNT  = 0x01;   // signed, 10 uV / LSB
const uint8_t  INA219_REG_BUS    = 0x02;   // bits 15..3, 4 mV / LSB
// BRNG=0 (16 V), PG=00 (+/-40 mV), BADC=1011 (8x avg), SADC=1111 (128x avg),
// MODE=111 (shunt + bus, continuous).
const uint16_t INA219_CONFIG     = 0x05FF;
bool inaPresent = false;

const int analogPin = A1;                  // legacy divider input (fallback)

// Below this -> no battery (v1.7.3: raised from 2000). On USB power with the
// battery switched off, ~4 V leaks back through the Nano regulator onto VIN,
// where the INA219 measures; 2 V read that as a nearly dead battery (CHARGE,
// 0 mA). A real pack never sits below 6 V: it holds ~9 V then cuts out, and the
// Nano cannot run from the battery below ~6.5 V anyway.
const int BATT_MV_NOBATT = 6000;
const int BATT_MV_LOW    = 7900;   // cliff warning (CHARGE + double-beep). Tune per pack.
const int BATT_MV_WARN   = 8500;   // early LOW warning before the cliff.

int  battCurrentmA    = 0;         // last measured device draw (INA219 only)
bool battCurrentValid = false;     // true when battCurrentmA came from the INA219

// ================= Coulomb-counting fuel gauge (INA219 only) =================
const uint16_t GAUGE_DEFAULT_CAP_MAH = 500;   // starting guess until a drain is learned
const uint16_t GAUGE_MIN_CAP_MAH     = 100;   // sanity clamps for learned capacity
const uint16_t GAUGE_MAX_CAP_MAH     = 3000;
const unsigned long GAUGE_SAMPLE_MS  = 500;   // integrate current every 0.5 s
const uint32_t GAUGE_SAVE_UAH        = 1000;  // persist every 1 mAh used (v1.7.4: was 2)
const uint8_t  GAUGE_CLIFF_SAMPLES   = 10;    // 10 x 0.5 s = 5 s below LOW to learn
const unsigned long GAUGE_FULL_HOLD_MS = 3000; // hold START on BATTERY page
const uint8_t  GAUGE_LOW_PCT         = 15;    // LOW warning at or below this %
const uint8_t  GAUGE_CHARGE_PCT      = 5;     // CHARGE warning at or below this %
const uint8_t  GAUGE_LEARN_MIN_PCT   = 70;    // FULL-mark learns if >= this % was used

// Off-page battery alarm (idle, any page except BATTERY, which has its own).
const unsigned long IDLE_LOW_BEEP_MS    = 60000;  // LOW: single beep every 60 s
const unsigned long IDLE_CHARGE_BEEP_MS = 30000;  // CHARGE: double beep every 30 s
int           gaugeLastMv     = 0;     // last bus voltage seen by gaugeTick
unsigned long idleAlarmTime   = 0;

// v1.7.3: quiet window for off-page alarms after user activity.
const unsigned long IDLE_QUIET_MS = 180000UL;    // 3 minutes
unsigned long lastUserActionMs    = 0;

// v1.7.3: FULL confirmation (hold START on BATTERY page -> STOP to confirm).
const unsigned long CONFIRM_TIMEOUT_MS = 5000;
bool          battConfirm        = false;
unsigned long battConfirmStart   = 0;
bool          confirmStartArmed  = false;   // START released since entering confirm
const unsigned long CONFIRM_ARM_MS   = 150;  // START released this long before it can cancel
const unsigned long CONFIRM_PRESS_MS = 40;   // a cancelling START press must last this long
unsigned long confirmReleasedSince   = 0;    // 0 = START not (yet) released
unsigned long confirmStartDownSince  = 0;    // 0 = START not pressed

// v1.7.3: STOP tap in idle is acted on at release (see normal STOP handler).
bool          stopTapPending = false;
unsigned long stopTapStart   = 0;

// START hold tracking (moved out of loop() so the confirm screen can reset it).
bool startWasPressedForHold = false;
bool startHoldUsed          = false;

// Corner gauge refresh while idle (redraw only when the % changes).
const unsigned long CORNER_REFRESH_MS = 10000;
unsigned long cornerRefreshTime = 0;
unsigned long idleBeepT       = 0;
uint8_t       idleBeepPhase   = 0;     // 0 idle, 1 beep on, 2 gap
uint8_t       idleBeepsLeft   = 0;

// Gauge flags (EEPROM byte)
const uint8_t GF_SYNCED  = 0x01;   // user has marked the pack FULL at least once
const uint8_t GF_ARMED   = 0x02;   // marked FULL since last learn -> next cliff is learnable
const uint8_t GF_LEARNED = 0x04;   // capacity has been learned at least once

uint16_t battCapMah    = GAUGE_DEFAULT_CAP_MAH;
uint8_t  gaugeFlags    = 0;
uint32_t usedUah       = 0;        // charge used since FULL, micro-amp-hours
uint32_t lastSavedUah  = 0;
uint32_t gaugeAcc      = 0;        // remainder, units of 0.1 mA * ms (36,000 = 1 uAh)
unsigned long gaugeLastMs = 0;
uint8_t  gaugeCliffCount = 0;
uint32_t gaugeSeq      = 0;        // wear-leveling sequence number
uint8_t  gaugeSlot     = 0;        // ring slot last written

// ================= Pin assignments =================
const uint8_t LED_PIN          = 13; // Shared white/green LED
const uint8_t BUZZER_PIN       = 3;  // PWM buzzer
const uint8_t START_BUTTON_PIN = 9;  // Button to start timer
const uint8_t STOP_BUTTON_PIN  = 8;  // Button to stop and cycle pages
const int     MIC_PIN          = A0; // Analog mic input

// ================= Sound detection parameters =================
const unsigned long SAMPLE_INTERVAL_US = 250; // ~4kHz sampling
const int WINDOW_SIZE = 20;                  // Samples per detection window

// Normal mode threshold (default); editable via hidden menu and saved to EEPROM.
const int DEFAULT_THRESHOLD = 150;       // used on first boot and after factory reset
int soundThresholdSetting = DEFAULT_THRESHOLD;

// Test mode threshold (0 so voice/noise will easily show activity)
const int SOUND_THRESHOLD_TEST = 0;

const unsigned long cooldownMs = 100;        // Minimum time between shots

// Shot timestamps. uint16_t keeps SRAM low; deltas use modular (wrap-safe)
// arithmetic, so a string up to ~65 s is timed correctly.
const int MAX_SHOTS = 60;
uint16_t shotTimes[MAX_SHOTS];

// ---- EEPROM persistence (ATmega328P has 1024 bytes of EEPROM) ----
// Saved splits live ONLY in EEPROM - deliberately no RAM copy (a 120-byte mirror
// pushed the stack into the OLED buffer and caused reboots mid-string).
const uint16_t EE_MAGIC      = 0x5713;   // signature marking valid saved data
const int EE_MAGIC_ADDR      = 0;        // uint16_t (2 bytes)
const int EE_THRESH_ADDR     = 2;        // int16_t  (2 bytes)
const int EE_SHOTCOUNT_ADDR  = 4;        // uint8_t  (1 byte); bytes 5..7 reserved
const int EE_SPLITS_ADDR     = 8;        // uint16_t[MAX_SHOTS] (120 bytes), ms splits

// Fuel-gauge block (bytes 128..263; never touched by firmware <= v1.7.0).
const uint16_t EE_GAUGE_MAGIC     = 0x6A19;
const int EE_GAUGE_MAGIC_ADDR     = 128;  // uint16_t
const int EE_GAUGE_CAP_ADDR       = 130;  // uint16_t learned capacity, mAh
const int EE_GAUGE_FLAGS_ADDR     = 132;  // uint8_t;  133..135 reserved
const int EE_GAUGE_RING_ADDR      = 136;  // GaugeRec[GAUGE_SLOTS]
const uint8_t GAUGE_SLOTS         = 16;
struct GaugeRec { uint32_t seq; uint32_t usedUah; };   // 8 bytes -> ring = 128 bytes

uint8_t  savedShotCount      = 0;        // count only (no RAM array of splits)
bool     showingSavedSession = false;    // true while review shows loaded (not live) data
bool     onSplash            = true;     // true while the boot splash is up

// Battery low-warning blink (animated only while the BATTERY page is shown + low).
bool          battWarnVisible = true;
unsigned long battBlinkTime   = 0;
const unsigned long BATT_BLINK_MS    = 450;
const unsigned long BATT_REFRESH_MS  = 1000;  // live voltage refresh when OK
unsigned long battRefreshTime = 0;

// Cliff buzzer: short double-beep every 5 s when below BATT_MV_LOW.
const unsigned long CLIFF_BUZZ_INTERVAL_MS = 5000;
const unsigned long CLIFF_BUZZ_ON_MS       = 80;   // each beep on-time
const unsigned long CLIFF_BUZZ_GAP_MS      = 120;  // gap between the two beeps
unsigned long cliffBuzzTime   = 0;
unsigned long cliffBeepTime   = 0;
uint8_t       cliffBeepPhase  = 0;   // 0=idle 1=beep1 2=gap 3=beep2 4=done

// LOW buzzer: single beep every 10 s when between BATT_MV_WARN and BATT_MV_LOW.
const unsigned long LOW_BUZZ_INTERVAL_MS = 10000;
const unsigned long LOW_BUZZ_ON_MS       = 80;
unsigned long lowBuzzTime  = 0;
bool          lowBeepActive = false;
unsigned long lowBeepStart  = 0;

// Factory reset: hold BOTH buttons in idle.
const unsigned long BOTH_HOLD_MS = 3000;
unsigned long bothHoldStartTime = 0;
bool          bothWasPressed    = false;

// Shots shown per review screen. 3 size-2 rows fill the 48 px blue band.
const int SHOTS_PER_PAGE = 3;

// ================= Random start delay =================
const unsigned long DELAY_MIN_MS = 3000;   // 3.000 s
const unsigned long DELAY_MAX_MS = 8000;   // 8.000 s
unsigned long delayDuration = 5000;        // chosen per run; default until first start

// ================= State machine =================
enum TimerState : uint8_t {
  STATE_IDLE    = 0,
  STATE_RUNNING = 2,
  STATE_DELAY   = 3
};
TimerState startState = STATE_IDLE;

unsigned long startPressTime = 0;
unsigned long delayStartTime = 0;
unsigned long runStartTime   = 0;
unsigned long lastEventTime  = 0;
bool lastLoudState = false;

int shotCount = 0;

// OLED paging
int currentPage = 0;

// Buzzer control
uint8_t buzzerVolume = 255;
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;

// LED blinking
unsigned long blinkTime = 0;
bool ledOn = false;

// ================= Hidden menu / test mode =================
bool testMode = false;
const unsigned long TEST_HOLD_MS = 2000; // Hold START this long (in idle) to enter test mode
int maxPeakTest = 0;                     // track max peak while test mode is up

// ================= Hidden menu / threshold setting =================
bool thresholdMenu = false;
const unsigned long THRESH_HOLD_MS = 2000; // Hold STOP this long (in idle) to enter threshold menu
unsigned long stopHoldStartTime = 0;
bool stopWasPressedForHold = false;
bool thresholdMenuExitArmed = false; // prevents immediate exit while still holding STOP to enter

// Threshold adjustment behavior
const int THRESH_MIN  = 0;
const int THRESH_MAX  = 500;
const int THRESH_STEP = 10; // base step

// Fast adjust behavior (hold START in threshold menu)
const unsigned long FAST_START_DELAY_MS = 350; // after holding this long, it speeds up
const unsigned long FAST_REPEAT_MS = 80;       // repeat period while holding
unsigned long startHoldStartTime_menu = 0;
unsigned long lastFastStepTime_menu = 0;
bool startHoldActive_menu = false;
bool startOneShotDone_menu = false;

// ================= Time formatting =================
// Print milliseconds as seconds with 2 decimals (e.g. 1215 -> "1.22").
void printSeconds(unsigned long ms) {
  unsigned long hundredths = (ms + 5) / 10;   // round to nearest 1/100 s
  unsigned long whole = hundredths / 100;
  unsigned int  frac  = hundredths % 100;
  display.print(whole);
  display.print('.');
  if (frac < 10) display.print('0');          // pad e.g. 0.05 not 0.5
  display.print(frac);
}

// Number of digits in the whole-seconds part of a time (1 for 0-9 s, 2, 3...).
uint8_t secondsDigits(unsigned long ms) {
  unsigned long whole = ((ms + 5) / 10) / 100;
  uint8_t d = 1;
  while (whole >= 10) { whole /= 10; d++; }
  return d;
}

// Draw "X.XXs" as large as fits, centered in the blue band. Handles strings of
// any length: size 4 up to 9.99 s, size 3 for 10.00 s and longer (3-digit seconds
// such as "104.37s" = 7 chars x 18 px = 126 px still fits the 128 px width).
void drawBigSecondsCentered(unsigned long ms, bool plus) {
  int chars = secondsDigits(ms) + 4 + (plus ? 1 : 0); // digits + ".XX" [+ "+"] + "s"
  int size = (chars * 24 <= SCREEN_WIDTH) ? 4 : 3;   // 6*size px per char
  int charW = 6 * size;
  int x = (SCREEN_WIDTH - chars * charW) / 2;
  if (x < 0) x = 0;
  int y = BLUE_TOP + (SCREEN_HEIGHT - BLUE_TOP - 7 * size) / 2; // center ink in blue band

  display.setTextSize(size);
  display.setCursor(x, y);
  printSeconds(ms);
  if (plus) display.print('+');
  display.print('s');
}

// Split ms for shot i (shot 0's "split" is the time from the start beep).
// 16-bit wrap-safe: correct as long as each single gap is under 65.5 s.
unsigned long shotSplit(int i) {
  if (i == 0) return (uint16_t)(shotTimes[0] - (uint16_t)runStartTime);
  return (uint16_t)(shotTimes[i] - shotTimes[i - 1]);
}

// v1.7.5: a gap longer than the 16-bit range is stored as exactly GAP_CAP_MS
// and shown as "65.53+" (a minimum), instead of wrapping to a wrong small value.
const uint16_t GAP_CAP_MS = 65530;
bool splitCapped(int i) { return shotSplit(i) >= GAP_CAP_MS; }
// True if any split up to and including shot i was capped (total is a minimum).
bool totalCapped(int i) {
  for (int k = 0; k <= i; k++) if (splitCapped(k)) return true;
  return false;
}

// v1.7.3: time from the beep to shot i, as a 32-bit sum of splits. Correct for
// strings of any length (only each single gap must stay under 65.5 s). Costs no
// RAM; at most 60 additions per call.
unsigned long shotTotal(int i) {
  unsigned long sum = 0;
  for (int k = 0; k <= i; k++) sum += shotSplit(k);
  return sum;
}

// ================= Corner battery gauge (v1.7.3) =================
int gaugePercent();                      // defined with the fuel gauge below
int cornerShownPct = -2;                 // last % drawn in a corner (-2 = none)

// Small battery icon + percentage in the top-right of the yellow strip.
// Stacked: icon on rows 0-6, "87%" on rows 8-15, right-aligned (x 104..127).
// INA219 units only; divider units keep the old layout.
void drawCornerGauge() {
  if (!inaPresent) return;
  int pct = gaugePercent();
  cornerShownPct = pct;
  const int ix = 109, iy = 0;
  display.drawRect(ix, iy, 11, 7, WHITE);        // body
  display.fillRect(ix + 11, iy + 2, 1, 3, WHITE); // nub
  if (pct > 0) {
    int fw = (9 * pct + 50) / 100;
    if (fw < 1) fw = 1;
    display.fillRect(ix + 1, iy + 1, fw, 5, WHITE);
  }
  display.setTextSize(1);
  if (pct < 0) {
    display.setCursor(SCREEN_WIDTH - 18, 8);
    display.print(F("--%"));
  } else {
    int chars = (pct >= 100) ? 4 : (pct >= 10) ? 3 : 2;
    display.setCursor(SCREEN_WIDTH - chars * 6, 8);
    display.print(pct);
    display.print('%');
  }
}

// ================= Review screens =================
// Screen 0: first-shot time, big and centered. "LAST STRING" (bottom) marks a
// string loaded from memory; the corner holds the battery gauge.
void drawFirstShotScreen() {
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("1ST SHOT"));
  drawCornerGauge();

  if (shotCount == 0) {
    display.setTextSize(2);
    display.setCursor(10, 36);
    display.print(F("NO SHOTS"));
  } else {
    drawBigSecondsCentered(shotSplit(0), splitCapped(0));
  }
  if (showingSavedSession) {
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 66) / 2, 56);
    display.print(F("LAST STRING"));
  }
}

// Screen 1: average split of all shots after the first (32-bit, any length).
void drawAvgSplitScreen() {
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("AVERAGE"));
  drawCornerGauge();

  if (shotCount >= 2) {
    unsigned long spanMs = shotTotal(shotCount - 1) - shotSplit(0);
    unsigned long avgMs = (spanMs + (shotCount - 1) / 2) / (shotCount - 1); // rounded
    bool plus = false;                       // a capped gap makes the average a minimum
    for (int k = 1; k < shotCount; k++) if (splitCapped(k)) { plus = true; break; }
    drawBigSecondsCentered(avgMs, plus);
  } else {
    display.setTextSize(4);
    display.setCursor(40, 26);
    display.print(F("--"));     // need >= 2 shots for a split
  }
}

// 3 shots per page, big font, in the blue band. cumulative=false -> splits.
void drawListScreen(int pageIndex, bool cumulative) {
  int pages = (shotCount + SHOTS_PER_PAGE - 1) / SHOTS_PER_PAGE;
  int startIndex = pageIndex * SHOTS_PER_PAGE;
  int endIndex = min(startIndex + SHOTS_PER_PAGE, shotCount);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(cumulative ? F("TOTALS") : F("SPLITS"));
  // Page number right after the 6-letter title (x 74); "20/20" ends at x 103,
  // clear of the corner gauge column at x 104.
  display.setTextSize(1);
  display.setCursor(74, 4);
  display.print(pageIndex + 1);
  display.print('/');
  display.print(pages);
  drawCornerGauge();

  display.setTextSize(2);
  for (int i = startIndex; i < endIndex; i++) {
    int y = BLUE_TOP + (i - startIndex) * 16;
    unsigned long v = cumulative ? shotTotal(i) : shotSplit(i);
    bool plus = cumulative ? totalCapped(i) : splitCapped(i);
    // Max 10 chars at size 2 (120 px of 128): drop the space after ':' if needed,
    // then the '#' (e.g. "12:104.09+").
    uint8_t numChars = (i + 1 >= 10) ? 2 : 1;
    uint8_t len = 1 + numChars + 1 + secondsDigits(v) + 3 + (plus ? 1 : 0);
    display.setCursor(0, y);
    if (len <= 10) display.print('#');
    display.print(i + 1);
    display.print(len + 1 <= 10 ? F(": ") : F(":"));
    printSeconds(v);
    if (plus) display.print('+');
  }
}

// ================= Battery sensing =================
// INA219 register helpers (big-endian 16-bit registers).
bool inaWriteReg(uint8_t reg, uint16_t val) {
  Wire.beginTransmission(INA219_ADDR);
  Wire.write(reg);
  Wire.write((uint8_t)(val >> 8));
  Wire.write((uint8_t)(val & 0xFF));
  return Wire.endTransmission() == 0;
}

bool inaReadReg(uint8_t reg, uint16_t &val) {
  Wire.beginTransmission(INA219_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(INA219_ADDR, (uint8_t)2) != 2) return false;
  uint8_t hi = Wire.read();     // read separately: evaluation order of
  uint8_t lo = Wire.read();     // (read()<<8)|read() is unspecified in C++
  val = ((uint16_t)hi << 8) | lo;
  return true;
}

// Probe + configure the INA219. Called once in setup() after the OLED starts Wire.
void initBatterySensor() {
  inaPresent = inaWriteReg(INA219_REG_CONFIG, INA219_CONFIG);
  DBG_PRINTLN(inaPresent ? F("Battery: INA219") : F("Battery: A1 divider"));
}

// Legacy divider: Vbatt = raw * 0.012075 + 0.23 V (8-sample average).
int readDividerMilliVolts() {
  long sum = 0;
  for (uint8_t i = 0; i < 8; i++) {
    sum += analogRead(analogPin);
    delayMicroseconds(200);   // let the S/H cap settle between samples
  }
  long raw = sum / 8;
  return (int)(raw * 12075L / 1000L + 230L);
}

// Battery-side voltage in mV from whichever source is active. Also updates
// battCurrentmA / battCurrentValid when the INA219 is used.
int readBatteryMilliVolts() {
  if (inaPresent) {
    uint16_t busReg, shuntReg;
    if (inaReadReg(INA219_REG_BUS, busReg) && inaReadReg(INA219_REG_SHUNT, shuntReg)) {
      int16_t sh = (int16_t)shuntReg;                 // 10 uV per LSB
      // 0.1 ohm shunt -> 0.1 mA per LSB; round to whole mA.
      battCurrentmA    = (sh >= 0) ? (sh + 5) / 10 : (sh - 5) / 10;
      battCurrentValid = true;
      // Bus voltage is measured at Vin- (load side); add the shunt drop to get
      // the battery-side (Vin+) voltage. sh/100 = shunt drop in mV.
      long mV = (long)(busReg >> 3) * 4L + sh / 100;
      return (int)mV;
    }
    // A failed read falls through to the divider for this sample.
  }
  battCurrentValid = false;
  return readDividerMilliVolts();
}

// ================= Fuel gauge =================
int gaugeRingAddr(uint8_t slot) {
  return EE_GAUGE_RING_ADDR + (int)slot * (int)sizeof(GaugeRec);
}

// Write the current charge-used total to the next ring slot (wear leveling).
void gaugeSaveUsed() {
  gaugeSeq++;
  gaugeSlot = (gaugeSlot + 1) % GAUGE_SLOTS;
  GaugeRec r = { gaugeSeq, usedUah };
  EEPROM.put(gaugeRingAddr(gaugeSlot), r);
  lastSavedUah = usedUah;
}

void gaugeSaveFlags() { EEPROM.put(EE_GAUGE_FLAGS_ADDR, gaugeFlags); }

// Load capacity, flags and the newest charge-used record. First boot on
// v1.7.1 lays down defaults (fresh EEPROM reads 0xFF, which would otherwise
// look like a huge sequence number).
void gaugeLoad() {
  uint16_t magic = 0xFFFF;
  EEPROM.get(EE_GAUGE_MAGIC_ADDR, magic);

  if (magic != EE_GAUGE_MAGIC) {
    battCapMah = GAUGE_DEFAULT_CAP_MAH;
    gaugeFlags = 0;
    usedUah = 0;
    gaugeSeq = 0;
    gaugeSlot = 0;
    GaugeRec blank = { 0, 0 };
    for (uint8_t i = 0; i < GAUGE_SLOTS; i++) EEPROM.put(gaugeRingAddr(i), blank);
    EEPROM.put(EE_GAUGE_CAP_ADDR, battCapMah);
    gaugeSaveFlags();
    EEPROM.put(EE_GAUGE_MAGIC_ADDR, (uint16_t)EE_GAUGE_MAGIC);
  } else {
    EEPROM.get(EE_GAUGE_CAP_ADDR, battCapMah);
    if (battCapMah < GAUGE_MIN_CAP_MAH || battCapMah > GAUGE_MAX_CAP_MAH)
      battCapMah = GAUGE_DEFAULT_CAP_MAH;
    EEPROM.get(EE_GAUGE_FLAGS_ADDR, gaugeFlags);

    // Newest record = highest sequence number.
    gaugeSeq = 0; gaugeSlot = 0; usedUah = 0;
    for (uint8_t i = 0; i < GAUGE_SLOTS; i++) {
      GaugeRec r;
      EEPROM.get(gaugeRingAddr(i), r);
      if (r.seq != 0xFFFFFFFFUL && r.seq >= gaugeSeq) {
        gaugeSeq = r.seq; gaugeSlot = i; usedUah = r.usedUah;
      }
    }
  }
  lastSavedUah = usedUah;
  // v1.7.4: charge used since the last save is lost at switch-off - anywhere
  // from 0 to one save interval, half an interval on average. Add that average
  // back so repeated power cycles don't leave the gauge reading high. It is not
  // written back until real use reaches the next save point, so switching on
  // and off without using the timer does not accumulate it.
  if ((gaugeFlags & GF_SYNCED) && usedUah > 0) usedUah += GAUGE_SAVE_UAH / 2;
}

// Store a measured capacity. First measurement replaces the 500 mAh default;
// later ones are blended 3:1 toward the newest so the estimate tracks aging.
void gaugeLearn(uint32_t usedMah) {
  if (usedMah < GAUGE_MIN_CAP_MAH) return;
  if (usedMah > GAUGE_MAX_CAP_MAH) usedMah = GAUGE_MAX_CAP_MAH;
  if (gaugeFlags & GF_LEARNED) battCapMah = (uint16_t)((battCapMah + 3UL * usedMah) / 4UL);
  else                         battCapMah = (uint16_t)usedMah;
  EEPROM.put(EE_GAUGE_CAP_ADDR, battCapMah);
  gaugeFlags |= GF_LEARNED;
  DBG_PRINT(F("Gauge learned capacity mAh: "));
  DBG_PRINTLN(battCapMah);
}

// User says the pack is freshly charged: reset to 100% and arm learning.
// If the discharge that just ended was never learned at a voltage cliff (a pack
// that holds 9 V until it shuts off) and it used >= 70% of the current estimate,
// treat it as a full drain: its saved count IS the real capacity.
void gaugeMarkFull() {
  if (gaugeFlags & GF_ARMED) {
    uint32_t usedMah = usedUah / 1000UL;
    if (usedMah * 100UL >= (uint32_t)battCapMah * GAUGE_LEARN_MIN_PCT) gaugeLearn(usedMah);
  }
  usedUah = 0;
  gaugeAcc = 0;
  gaugeCliffCount = 0;
  gaugeFlags |= (GF_SYNCED | GF_ARMED);
  gaugeSaveFlags();
  gaugeSaveUsed();
}

// Remaining charge 1..100 %, or -1 if the pack has never been marked FULL.
// Floors at 1% if usage runs past the estimate (estimate was low -> stays in
// CHARGE warning until the pack dies, and the next FULL mark corrects it).
int gaugePercent() {
  if (!(gaugeFlags & GF_SYNCED)) return -1;
  uint32_t usedPct = usedUah / ((uint32_t)battCapMah * 10UL);   // uAh -> % of cap
  if (usedPct >= 99) return 1;
  return 100 - (int)usedPct;
}

// Combined battery state: the worse of the voltage thresholds and (when the
// gauge is valid) the percentage thresholds.
enum BattState : uint8_t { BS_OK = 0, BS_LOW = 1, BS_CHARGE = 2 };

uint8_t battStateFor(int mV) {
  // No battery (switched off, or running on USB only): nothing to warn about,
  // even if the gauge's saved percentage is low.
  if (mV < BATT_MV_NOBATT) return BS_OK;

  uint8_t st = BS_OK;
  if (mV < BATT_MV_LOW)       st = BS_CHARGE;
  else if (mV < BATT_MV_WARN) st = BS_LOW;
  if (inaPresent) {
    int pct = gaugePercent();
    if (pct >= 0) {
      if (pct <= GAUGE_CHARGE_PCT)                 st = BS_CHARGE;
      else if (pct <= GAUGE_LOW_PCT && st < BS_LOW) st = BS_LOW;
    }
  }
  return st;
}

// Called every loop. Every GAUGE_SAMPLE_MS: read current, integrate charge,
// persist periodically, and learn capacity when a FULL-marked pack hits the cliff.
void gaugeTick(unsigned long now) {
  if (!inaPresent) return;
  unsigned long dt = now - gaugeLastMs;
  if (dt < GAUGE_SAMPLE_MS) return;
  gaugeLastMs = now;
  if (dt > 60000UL) dt = 60000UL;            // guard against a long stall

  uint16_t busReg, shuntReg;
  if (!inaReadReg(INA219_REG_BUS, busReg) || !inaReadReg(INA219_REG_SHUNT, shuntReg)) return;
  int16_t sh = (int16_t)shuntReg;             // 0.1 mA per LSB on the 0.1 ohm shunt
  long mV = (long)(busReg >> 3) * 4L + sh / 100;
  gaugeLastMv = (int)mV;                      // cached for the off-page alarm
  if (mV < BATT_MV_NOBATT) return;            // on USB / no battery: nothing to count

  if (sh > 0) {
    gaugeAcc += (uint32_t)sh * dt;            // <= 4000 * 60000, fits in uint32
    usedUah  += gaugeAcc / 36000UL;           // 36,000 (0.1 mA * ms) = 1 uAh
    gaugeAcc %= 36000UL;
  }
  if (usedUah - lastSavedUah >= GAUGE_SAVE_UAH) gaugeSaveUsed();

  // Learn capacity at the cliff (debounced so a buzzer sag can't trigger it).
  if ((gaugeFlags & GF_ARMED) && mV < BATT_MV_LOW) {
    if (++gaugeCliffCount >= GAUGE_CLIFF_SAMPLES) {
      gaugeLearn(usedUah / 1000UL);
      gaugeFlags &= ~GF_ARMED;                // one learn per FULL mark
      gaugeSaveFlags();
      gaugeSaveUsed();
    }
  } else {
    gaugeCliffCount = 0;
  }
}

// v1.7.3: "MARK FULL?" prompt shown after holding START on the BATTERY page.
void drawFullConfirmPrompt() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("BATTERY"));
  display.setCursor(4, 20);
  display.print(F("MARK FULL?"));
  display.setTextSize(1);
  display.setCursor(0, 44);
  display.print(F("STOP  = yes"));
  display.setCursor(0, 54);
  display.print(F("START = no"));
  display.display();
}

// "FULL" confirmation after the hold-START gesture on the BATTERY page.
void showGaugeFullConfirm() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("BATTERY"));
  display.setTextSize(3);
  display.setCursor(28, 22);
  display.print(F("FULL"));
  display.setTextSize(1);
  display.setCursor(16, 52);
  display.print(F("Gauge set to 100%"));
  display.display();
  delay(900);
}

// Print volts as "X.XX" from millivolts (integer math, no float lib).
void printVolts(int mV) {
  if (mV < 0) mV = 0;
  int whole = mV / 1000;
  int frac  = (mV % 1000) / 10;       // hundredths of a volt
  display.print(whole);
  display.print('.');
  if (frac < 10) display.print('0');
  display.print(frac);
}

// Battery outline icon in the blue band; filled when healthy, empty when low.
void drawBatteryIcon(bool filled) {
  display.drawRect(2, 50, 116, 12, WHITE);   // body outline (x 2..117)
  display.fillRect(118, 53, 3, 6, WHITE);    // terminal nub
  if (filled) display.fillRect(4, 52, 112, 8, WHITE);
}

void drawBatteryScreen() {
  int mV = readBatteryMilliVolts();
  bool haveBatt = (mV >= BATT_MV_NOBATT);

  // Title (yellow band)
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("BATTERY"));

  // Live voltage (and current, with INA219) - yellow band, right side.
  if (haveBatt) {
    display.setTextSize(1);
    display.setCursor(92, 0);
    printVolts(mV);
    display.print('V');
    if (battCurrentValid) {
      display.setCursor(92, 8);
      display.print(battCurrentmA);
      display.print(F("mA"));
    }
  }

  if (!haveBatt) {
    display.setTextSize(2);
    display.setCursor(4, 34);
    display.print(F("NO BATTERY"));
    return;
  }

  uint8_t st = battStateFor(mV);
  bool low  = (st == BS_CHARGE);
  bool warn = (st == BS_LOW);
  int  gpct = battCurrentValid ? gaugePercent() : -1;   // -1 = no gauge

  if (!low && !warn) {
    if (!battCurrentValid) {
      // Divider-only unit: classic OK screen.
      display.setTextSize(3);
      display.setCursor(46, 22);
      display.print(F("OK"));
      drawBatteryIcon(true);
    } else {
      int pct = gaugePercent();
      if (pct < 0) {
        // Never marked FULL: prompt the user.
        display.setTextSize(3);
        display.setCursor(37, 20);
        display.print(F("--%"));
        display.setTextSize(1);
        display.setCursor(10, 42);
        display.print(F("Hold START if full"));
        drawBatteryIcon(false);
      } else {
        // Big centered percentage + proportional fill.
        int chars = (pct >= 100) ? 4 : (pct >= 10) ? 3 : 2;   // digits + '%'
        display.setTextSize(3);
        display.setCursor((SCREEN_WIDTH - chars * 18) / 2, 22);
        display.print(pct);
        display.print('%');
        drawBatteryIcon(false);
        int fillW = (112 * pct) / 100;
        if (fillW < 1) fillW = 1;
        display.fillRect(4, 52, fillW, 8, WHITE);
      }
    }
  } else if (warn) {
    if (battWarnVisible) {
      display.setTextSize(3);
      display.setCursor(28, 22);
      display.print(F("LOW"));
    }
    drawBatteryIcon(false);
    if (gpct >= 0) {
      // Steady percentage beside the flashing LOW, fill matches it.
      display.setTextSize(2);
      display.setCursor(88, 26);
      display.print(gpct);
      display.print('%');
      int fillW = (112 * gpct) / 100;
      if (fillW < 1) fillW = 1;
      display.fillRect(4, 52, fillW, 8, WHITE);
    } else {
      display.fillRect(4, 52, 56, 8, WHITE);   // divider unit: half-filled
    }
  } else {
    if (gpct >= 0) {
      // v1.7.5: flashing CHARGE at size 2 with a steady percentage beside it, in
      // the same spot as on the LOW screen.
      if (battWarnVisible) {
        display.setTextSize(2);
        display.setCursor(8, 26);
        display.print(F("CHARGE"));
      }
      display.setTextSize(2);
      display.setCursor(88, 26);
      display.print(gpct);
      display.print('%');
      drawBatteryIcon(false);
      int fillW = (112 * gpct) / 100;
      if (fillW < 1) fillW = 1;
      display.fillRect(4, 52, fillW, 8, WHITE);
    } else {
      // Divider unit (no gauge): large flashing CHARGE as before.
      if (battWarnVisible) {
        display.setTextSize(3);
        display.setCursor(4, 22);
        display.print(F("CHARGE"));
      }
      drawBatteryIcon(false);
    }
  }
}

// ================= OLED display (page router) =================
void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  if (startState == STATE_RUNNING && shotCount == 0) {
    display.setTextSize(3);
    display.setCursor(37, 24);
    display.print(F("GO!"));
    display.display();
    return;
  }

  int splitPages = (shotCount + SHOTS_PER_PAGE - 1) / SHOTS_PER_PAGE; // 0 if no shots

  if (currentPage == 0) {
    drawFirstShotScreen();
  } else if (currentPage == 1) {
    drawAvgSplitScreen();
  } else if (currentPage <= 1 + splitPages) {
    drawListScreen(currentPage - 2, false);                   // SPLITS
  } else if (currentPage <= 1 + 2 * splitPages) {
    drawListScreen(currentPage - 2 - splitPages, true);       // TOTALS
  } else {
    drawBatteryScreen();
  }

  display.display();
}

int reviewTotalPages() {
  int splitPages = (shotCount + SHOTS_PER_PAGE - 1) / SHOTS_PER_PAGE;
  return 3 + 2 * splitPages;   // 1st shot + avg split + splits + totals + battery
}

int livePageForLatestShot() {
  return 2 + (shotCount - 1) / SHOTS_PER_PAGE;
}

// ================= Test screen =================
int freeRam() {
#ifdef __AVR__
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
#else
  return -1;
#endif
}

void drawTestScreen(int peak, int maxPeak) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(F("HIDDEN TEST MODE"));

  // Battery source - quick production check that the INA219 is detected.
  display.setCursor(0, 8);
  display.print(inaPresent ? F("BATT:INA219") : F("BATT:DIVIDER"));
  if (inaPresent) {
    // Charge used / capacity, mAh ("*" once capacity has been learned).
    display.setCursor(70, 8);
    display.print(usedUah / 1000UL);
    display.print('/');
    display.print(battCapMah);
    if (gaugeFlags & GF_LEARNED) display.print('*');
  }

  display.setCursor(70, 20);
  display.print(F("THRESH="));
  display.print(SOUND_THRESHOLD_TEST);

  display.setCursor(0, 20);
  display.print(F("PEAK="));
  display.print(peak);

  display.setCursor(0, 30);
  display.print(F("MAX="));
  display.print(maxPeak);

  display.setCursor(70, 30);
  display.print(F("RAM="));
  display.print(freeRam());

  display.setCursor(0, 42);
  display.println(F("Speak/clap near mic"));
  display.setCursor(0, 54);
  display.println(F("STOP = exit"));

  display.display();
}

// ================= Threshold menu screen =================
void drawThresholdMenu() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("SET SOUND THRESHOLD"));

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print(F("TH="));
  display.print(soundThresholdSetting);

  display.setTextSize(1);
  display.setCursor(0, 44);
  display.println(F("START +10  hold:FAST"));   // 20 chars = 120 px, fits 128
  display.setCursor(0, 54);
  display.println(F("STOP: exit"));

  display.display();
}

// ================= Sound detection =================
int measurePeakToPeak() {
  unsigned long t0 = micros();
  int maxV = 0, minV = 1023;

  for (int i = 0; i < WINDOW_SIZE; i++) {
    int v = analogRead(MIC_PIN);
    maxV = max(maxV, v);
    minV = min(minV, v);
    while (micros() - t0 < (unsigned long)(i + 1) * SAMPLE_INTERVAL_US);
  }
  return maxV - minV;
}

// ================= EEPROM persistence + boot helpers =================
void loadSettings() {
  uint16_t magic = 0xFFFF;
  EEPROM.get(EE_MAGIC_ADDR, magic);

  if (magic == EE_MAGIC) {
    int16_t th = DEFAULT_THRESHOLD;
    EEPROM.get(EE_THRESH_ADDR, th);
    if (th < THRESH_MIN || th > THRESH_MAX) th = DEFAULT_THRESHOLD;
    soundThresholdSetting = th;

    uint8_t cnt = 0;
    EEPROM.get(EE_SHOTCOUNT_ADDR, cnt);
    if (cnt > MAX_SHOTS) cnt = 0;
    savedShotCount = cnt;
  } else {
    soundThresholdSetting = DEFAULT_THRESHOLD;
    savedShotCount = 0;
    EEPROM.put(EE_THRESH_ADDR, (int16_t)soundThresholdSetting);
    EEPROM.put(EE_SHOTCOUNT_ADDR, (uint8_t)0);
    EEPROM.put(EE_MAGIC_ADDR, (uint16_t)EE_MAGIC);
  }
}

void loadSavedSessionIntoLive() {
  runStartTime = 0;
  uint16_t cumulative = 0;
  for (uint8_t i = 0; i < savedShotCount; i++) {
    uint16_t sp = 0;
    EEPROM.get(EE_SPLITS_ADDR + (int)i * (int)sizeof(uint16_t), sp);
    cumulative += sp;
    shotTimes[i] = cumulative;
  }
  shotCount = savedShotCount;
  showingSavedSession = (savedShotCount > 0);
}

void saveThreshold() {
  EEPROM.put(EE_THRESH_ADDR, (int16_t)soundThresholdSetting);
}

void saveSession() {
  uint8_t cnt = (shotCount > MAX_SHOTS) ? (uint8_t)MAX_SHOTS : (uint8_t)shotCount;
  for (uint8_t i = 0; i < cnt; i++) {
    uint16_t sp = (uint16_t)shotSplit(i);
    EEPROM.put(EE_SPLITS_ADDR + (int)i * (int)sizeof(uint16_t), sp);
  }
  savedShotCount = cnt;
  EEPROM.put(EE_SHOTCOUNT_ADDR, cnt);
  EEPROM.put(EE_MAGIC_ADDR, (uint16_t)EE_MAGIC);
}

void showSplash() {
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("Shot Timer Mk1"));      // yellow strip, line 1
  display.setCursor(0, 8);
  display.print(F("v1.7.5"));              // yellow strip, line 2
  drawCornerGauge();                       // same corner gauge as every page

  display.setTextSize(2);
  int16_t startWidth = 6 * 12;
  display.setCursor((SCREEN_WIDTH - startWidth) / 2 + 6, 16);
  display.println(F("START"));

  int16_t readyWidth = 11 * 12;
  display.setCursor((SCREEN_WIDTH - readyWidth) / 2 + 6, 40);
  display.println(F("when ready"));

  if (savedShotCount > 0) {
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print(F("STOP: last results"));
  }
  display.display();
}

// Battery alarm for every idle page except BATTERY (which runs its own faster
// cadence in maybeBlinkBattery). Uses the voltage cached by gaugeTick, so it
// adds no I2C traffic. Never sounds during the start delay, a run, test mode or
// the threshold menu. INA219 builds only.
void stopIdleBeep() {
  if (idleBeepPhase != 0) analogWrite(BUZZER_PIN, 0);
  idleBeepPhase = 0;
  idleBeepsLeft = 0;
}

void idleBattAlarm(unsigned long now) {
  if (!inaPresent || testMode || thresholdMenu || battConfirm ||
      startState != STATE_IDLE) {
    stopIdleBeep();
    return;
  }
  // v1.7.3: stay quiet while the user is actively using the timer. Any button
  // press (or the end of a string) restarts a 3-minute quiet window; the corner
  // gauge shows the percentage in the meantime.
  if (now - lastUserActionMs < IDLE_QUIET_MS) {
    stopIdleBeep();
    return;
  }
  if (!onSplash && currentPage == reviewTotalPages() - 1) {   // BATTERY page
    stopIdleBeep();
    return;
  }

  if (idleBeepPhase == 0) {
    uint8_t st = battStateFor(gaugeLastMv);
    if (st == BS_OK) return;
    unsigned long interval = (st == BS_CHARGE) ? IDLE_CHARGE_BEEP_MS : IDLE_LOW_BEEP_MS;
    if (idleAlarmTime != 0 && now - idleAlarmTime < interval) return;
    idleAlarmTime = now;
    idleBeepsLeft = (st == BS_CHARGE) ? 2 : 1;
    idleBeepPhase = 1;
    idleBeepT     = now;
    analogWrite(BUZZER_PIN, 200);
  } else if (idleBeepPhase == 1) {
    if (now - idleBeepT >= CLIFF_BUZZ_ON_MS) {
      analogWrite(BUZZER_PIN, 0);
      idleBeepT = now;
      idleBeepPhase = (--idleBeepsLeft > 0) ? 2 : 0;
    }
  } else if (idleBeepPhase == 2) {
    if (now - idleBeepT >= CLIFF_BUZZ_GAP_MS) {
      analogWrite(BUZZER_PIN, 200);
      idleBeepT = now;
      idleBeepPhase = 1;
    }
  }
}

// v1.7.3: keep the corner gauge current while a page sits idle. Checks every
// 10 s and redraws only when the percentage has changed (about every 9-10 min),
// never during the start delay or a run. The BATTERY page refreshes itself.
void cornerGaugeRefresh(unsigned long now) {
  if (!inaPresent || testMode || thresholdMenu || battConfirm ||
      startState != STATE_IDLE) return;
  if (!onSplash && currentPage == reviewTotalPages() - 1) return;
  if (now - cornerRefreshTime < CORNER_REFRESH_MS) return;
  cornerRefreshTime = now;
  if (gaugePercent() == cornerShownPct) return;
  if (onSplash) showSplash(); else updateDisplay();
}

void factoryReset() {
  soundThresholdSetting = DEFAULT_THRESHOLD;
  savedShotCount = 0;
  shotCount = 0;
  showingSavedSession = false;

  EEPROM.put(EE_THRESH_ADDR, (int16_t)soundThresholdSetting);
  EEPROM.put(EE_SHOTCOUNT_ADDR, (uint8_t)0);
  EEPROM.put(EE_MAGIC_ADDR, (uint16_t)EE_MAGIC);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(10, 4);
  display.print(F("RESET"));
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println(F("Saved data cleared"));
  display.print(F("Threshold = "));
  display.println(DEFAULT_THRESHOLD);
  display.display();
  delay(1200);

  startState = STATE_IDLE;
  currentPage = 0;
  onSplash = true;
  digitalWrite(LED_PIN, LOW);
  showSplash();
}

// Animate battery warnings and fire buzzers while BATTERY page is shown idle.
void maybeBlinkBattery(unsigned long now) {
  if (testMode || thresholdMenu || startState != STATE_IDLE || onSplash) return;
  if (currentPage != reviewTotalPages() - 1) return;   // BATTERY is the last page

  int mV = readBatteryMilliVolts();
  bool haveBatt = (mV >= BATT_MV_NOBATT);
  uint8_t st = haveBatt ? battStateFor(mV) : BS_OK;
  bool low  = (st == BS_CHARGE);
  bool warn = (st == BS_LOW);

  if (!low && !warn) {
    battWarnVisible = true;
    if (now - battRefreshTime >= BATT_REFRESH_MS) {
      battRefreshTime = now;
      updateDisplay();
    }
    cliffBeepPhase = 0;
    lowBeepActive  = false;
    analogWrite(BUZZER_PIN, 0);
    return;
  }

  if (now - battBlinkTime >= BATT_BLINK_MS) {
    battBlinkTime = now;
    battWarnVisible = !battWarnVisible;
    updateDisplay();
  }

  if (warn) {
    if (!lowBeepActive && (now - lowBuzzTime >= LOW_BUZZ_INTERVAL_MS)) {
      lowBuzzTime   = now;
      lowBeepStart  = now;
      lowBeepActive = true;
      analogWrite(BUZZER_PIN, 200);
    }
    if (lowBeepActive && (now - lowBeepStart >= LOW_BUZZ_ON_MS)) {
      lowBeepActive = false;
      analogWrite(BUZZER_PIN, 0);
    }
    cliffBeepPhase = 0;
    return;
  }

  // CHARGE (cliff): non-blocking double-beep every 5 s.
  if (cliffBeepPhase == 0) {
    if (now - cliffBuzzTime >= CLIFF_BUZZ_INTERVAL_MS) {
      cliffBuzzTime  = now;
      cliffBeepTime  = now;
      cliffBeepPhase = 1;
      analogWrite(BUZZER_PIN, 200);
    }
  } else if (cliffBeepPhase == 1) {
    if (now - cliffBeepTime >= CLIFF_BUZZ_ON_MS) {
      cliffBeepTime  = now;
      cliffBeepPhase = 2;
      analogWrite(BUZZER_PIN, 0);
    }
  } else if (cliffBeepPhase == 2) {
    if (now - cliffBeepTime >= CLIFF_BUZZ_GAP_MS) {
      cliffBeepTime  = now;
      cliffBeepPhase = 3;
      analogWrite(BUZZER_PIN, 200);
    }
  } else if (cliffBeepPhase == 3) {
    if (now - cliffBeepTime >= CLIFF_BUZZ_ON_MS) {
      cliffBeepPhase = 4;
      analogWrite(BUZZER_PIN, 0);
    }
  }
  if (cliffBeepPhase == 4 && (now - cliffBuzzTime >= CLIFF_BUZZ_INTERVAL_MS)) {
    cliffBeepPhase = 0;
  }
}

// ================= Setup =================
void setup() {
  DBG_BEGIN();
  delay(100); // Allow OLED (and INA219) to power up

  DBG_PRINTLN(F("Initializing OLED..."));
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    DBG_PRINTLN(F("OLED failed to initialize."));
    while (true);
  }

  // 400 kHz fast-mode I2C (INA219 supports up to 2.56 MHz, so it's fine here).
  Wire.setClock(400000UL);

  // Detect INA219; falls back to the A1 divider if it doesn't answer.
  initBatterySensor();
  gaugeLoad();
  gaugeLastMs = millis();

  loadSettings();
  loadSavedSessionIntoLive();

  onSplash = true;
  showSplash();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 0);

  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);

}

void stepThresholdOnce() {
  soundThresholdSetting += THRESH_STEP;
  if (soundThresholdSetting > THRESH_MAX) soundThresholdSetting = THRESH_MIN;
  drawThresholdMenu();
}

// ================= Main loop =================
void loop() {
  unsigned long now = millis();

  // Fuel gauge runs on every pass, before any menu/mode returns early.
  gaugeTick(now);
  idleBattAlarm(now);

  // ---- v1.7.3: FULL confirmation prompt (takes over the buttons) ----
  if (battConfirm) {
    bool startDown = (digitalRead(START_BUTTON_PIN) == LOW);
    bool stopDown  = (digitalRead(STOP_BUTTON_PIN)  == LOW);

    // v1.7.4: START must stay released for CONFIRM_ARM_MS before a START tap can
    // cancel. Contact bounce when letting go of the 3 s hold used to look like a
    // new press and cancelled the prompt instantly. The 5 s timeout also starts
    // only once START is released.
    if (!confirmStartArmed) {
      if (startDown) {
        confirmReleasedSince = 0;
      } else {
        if (confirmReleasedSince == 0) confirmReleasedSince = now ? now : 1;
        if (now - confirmReleasedSince >= CONFIRM_ARM_MS) {
          confirmStartArmed = true;
          battConfirmStart = now;               // timeout counts from here
        }
      }
    }

    // A cancelling START press must be held CONFIRM_PRESS_MS (not a glitch).
    bool startCancel = false;
    if (confirmStartArmed && startDown) {
      if (confirmStartDownSince == 0) confirmStartDownSince = now ? now : 1;
      if (now - confirmStartDownSince >= CONFIRM_PRESS_MS) startCancel = true;
    } else {
      confirmStartDownSince = 0;
    }

    if (stopDown) {
      delay(30);                                 // simple debounce
      if (digitalRead(STOP_BUTTON_PIN) == LOW) {
        gaugeMarkFull();
        showGaugeFullConfirm();
        while (digitalRead(STOP_BUTTON_PIN) == LOW) { /* wait for release */ }
        battConfirm = false;
      }
    } else if (startCancel ||
               (confirmStartArmed && now - battConfirmStart >= CONFIRM_TIMEOUT_MS)) {
      while (digitalRead(START_BUTTON_PIN) == LOW) { /* wait for release */ }
      battConfirm = false;                       // cancelled: nothing changes
    }

    if (!battConfirm) {
      // Clean button state so the next tap behaves normally.
      startWasPressedForHold = false;
      startHoldUsed = false;
      stopWasPressedForHold = false;
      lastUserActionMs = millis();
      delay(60);                                 // let contacts settle
      updateDisplay();                           // back to the BATTERY page
    }
    return;
  }

  bool startPressed = (digitalRead(START_BUTTON_PIN) == LOW);
  bool stopPressed  = (digitalRead(STOP_BUTTON_PIN)  == LOW);
  if (startPressed || stopPressed) lastUserActionMs = now;   // quiet-window timer

  // ================= Factory reset: hold BOTH buttons in IDLE =================
  if (!testMode && !thresholdMenu && startState == STATE_IDLE) {
    if (startPressed && stopPressed) {
      if (!bothWasPressed) { bothWasPressed = true; bothHoldStartTime = now; }
      if ((now - bothHoldStartTime) >= BOTH_HOLD_MS) {
        factoryReset();
        bothWasPressed = false;
        while (digitalRead(START_BUTTON_PIN) == LOW ||
               digitalRead(STOP_BUTTON_PIN)  == LOW) { /* hold */ }
        return;
      }
      delay(10);
      return;
    } else {
      bothWasPressed = false;
    }
  }

  // ================= Hidden menu entry: hold STOP in IDLE =================
  if (!testMode && !thresholdMenu && startState == STATE_IDLE) {
    if (stopPressed && !stopWasPressedForHold) {
      stopWasPressedForHold = true;
      stopHoldStartTime = now;
    }

    if (stopPressed && stopWasPressedForHold) {
      if ((now - stopHoldStartTime) >= THRESH_HOLD_MS) {
        thresholdMenu = true;
        thresholdMenuExitArmed = false;
        digitalWrite(LED_PIN, HIGH);
        startHoldActive_menu = false;
        startOneShotDone_menu = false;
        drawThresholdMenu();
      }
    }

    if (!stopPressed && stopWasPressedForHold) {
      stopWasPressedForHold = false;
    }
  }

  // ================= Threshold menu behavior =================
  if (thresholdMenu) {
    if (!stopPressed) thresholdMenuExitArmed = true;

    if (startPressed) {
      if (!startHoldActive_menu) {
        startHoldActive_menu = true;
        startHoldStartTime_menu = now;
        lastFastStepTime_menu = now;
        startOneShotDone_menu = false;
      }

      if (!startOneShotDone_menu) {
        stepThresholdOnce();
        startOneShotDone_menu = true;
      }

      if ((now - startHoldStartTime_menu) >= FAST_START_DELAY_MS) {
        if ((now - lastFastStepTime_menu) >= FAST_REPEAT_MS) {
          lastFastStepTime_menu = now;
          stepThresholdOnce();
        }
      }
    } else {
      startHoldActive_menu = false;
      startOneShotDone_menu = false;
    }

    // v1.7.3: exit on a STOP press, then wait for release and clear the hold
    // tracker. Previously the exit tap was still held on the next pass and the
    // stale "hold STOP 2 s" timer reopened the menu straight away.
    if (thresholdMenuExitArmed && digitalRead(STOP_BUTTON_PIN) == LOW) {
      delay(15);                                   // reject a contact glitch
      if (digitalRead(STOP_BUTTON_PIN) == LOW) {
        while (digitalRead(STOP_BUTTON_PIN) == LOW) { /* wait for release */ }
        delay(30);                                 // let the contacts settle
        thresholdMenu = false;
        stopWasPressedForHold = false;
        stopTapPending = false;
        saveThreshold();
        digitalWrite(LED_PIN, LOW);
        onSplash = false;
        currentPage = 0;
        lastUserActionMs = millis();
        updateDisplay();
        return;
      }
    }

    delay(10);
    return;
  }

  // ================= Test mode entry: hold START in IDLE =================

  if (!testMode && startState == STATE_IDLE) {
    // On the BATTERY page (with INA219), holding START marks the pack FULL
    // instead of opening test mode.
    bool onBattPage = inaPresent && !onSplash &&
                      (currentPage == reviewTotalPages() - 1);

    if (startPressed && !startWasPressedForHold) {
      startPressTime = now;
      startWasPressedForHold = true;
      startHoldUsed = false;
    }

    if (startPressed && startWasPressedForHold && !startHoldUsed) {
      if (onBattPage) {
        if ((now - startPressTime) >= GAUGE_FULL_HOLD_MS) {
          startHoldUsed = true;            // releasing START must not start a run
          battConfirm = true;
          battConfirmStart = now;
          confirmStartArmed = false;
          confirmReleasedSince = 0;
          confirmStartDownSince = 0;
          drawFullConfirmPrompt();
        }
      } else if ((now - startPressTime) >= TEST_HOLD_MS) {
        testMode = true;
        startHoldUsed = true;            // v1.7.4: releasing this hold must not start a run
        maxPeakTest = 0;
        digitalWrite(LED_PIN, HIGH);
      }
    }

    if (!startPressed && startWasPressedForHold) {
      startWasPressedForHold = false;

      if (!testMode && !startHoldUsed) {
        randomSeed(micros());
        delayDuration = random(DELAY_MIN_MS, DELAY_MAX_MS + 1);

        showingSavedSession = false;
        onSplash = false;

        startState = STATE_DELAY;
        delayStartTime = now;
        digitalWrite(LED_PIN, HIGH);
      }
    }
  }

  // ================= Test mode loop =================
  if (testMode) {
    int peak = measurePeakToPeak();
    if (peak > maxPeakTest) maxPeakTest = peak;

    drawTestScreen(peak, maxPeakTest);
    digitalWrite(LED_PIN, HIGH);

    // v1.7.3: responsive exit. Each pass takes ~60 ms (mic window + full screen
    // redraw + pause), so a quick STOP tap could fall between checks. Watch
    // STOP continuously through the pause and exit on the first solid press.
    bool stopTapped = false;
    unsigned long waitStart = millis();
    while (millis() - waitStart < 30) {
      if (digitalRead(STOP_BUTTON_PIN) == LOW) { stopTapped = true; break; }
    }
    if (!stopTapped && digitalRead(STOP_BUTTON_PIN) == LOW) stopTapped = true;

    if (stopTapped) {
      delay(15);                                   // reject a contact glitch
      if (digitalRead(STOP_BUTTON_PIN) == LOW) {
        while (digitalRead(STOP_BUTTON_PIN) == LOW) { /* wait for release */ }
        delay(30);                                 // let the contacts settle
        testMode = false;
        startState = STATE_IDLE;
        startWasPressedForHold = false;  // v1.7.4: forget the hold that opened test mode
        startHoldUsed = false;
        digitalWrite(LED_PIN, LOW);
        buzzerActive = false;
        analogWrite(BUZZER_PIN, 0);
        lastLoudState = false;
        onSplash = false;
        currentPage = 0;
        lastUserActionMs = millis();
        updateDisplay();
      }
    }
    return;
  }

  // ================= Normal STOP behavior with debounce =================
  static bool stopButtonLastState_local = HIGH;
  static bool stopButtonStableState_local = HIGH;
  static unsigned long stopButtonLastChangeTime_local = 0;
  const unsigned long debounceDelay_local = 50;

  bool stopReading = digitalRead(STOP_BUTTON_PIN);

  if (stopReading != stopButtonLastState_local) {
    stopButtonLastChangeTime_local = now;
  }
  stopButtonLastState_local = stopReading;

  if ((now - stopButtonLastChangeTime_local) > debounceDelay_local) {
    if (stopReading != stopButtonStableState_local) {
      stopButtonStableState_local = stopReading;

      if (stopButtonStableState_local == LOW) {
        // ---- STOP pressed ----
        if (startState == STATE_RUNNING) {
#if DEBUG_SERIAL
          Serial.println(F("\nSplit Time Table:"));
          for (int i = 0; i < shotCount; i++) {
            Serial.print(F("#"));
            Serial.print(i + 1);
            Serial.print(F(": "));
            Serial.println(shotSplit(i));
          }
#endif

          saveSession();
          // v1.7.4: also bank the gauge count at the end of every string, so a
          // short session followed by switch-off loses nothing.
          if (inaPresent && usedUah != lastSavedUah) gaugeSaveUsed();

          startState = STATE_IDLE;
          showingSavedSession = false;
          onSplash = false;
          digitalWrite(LED_PIN, LOW);
          buzzerActive = false;
          analogWrite(BUZZER_PIN, 0);
          lastLoudState = false;
          currentPage = 0;
          stopTapPending = false;        // the release must not also flip a page
          updateDisplay();
        } else if (startState == STATE_IDLE) {
          // v1.7.3: in idle, act on RELEASE so a 2 s hold (sensitivity menu)
          // or the 3 s two-button reset no longer flips a page first.
          stopTapPending = true;
          stopTapStart = now;
        }
      } else {
        // ---- STOP released ----
        if (stopTapPending && startState == STATE_IDLE &&
            (now - stopTapStart) < THRESH_HOLD_MS) {
          if (onSplash) {
            onSplash = false;
            currentPage = 0;
          } else {
            int totalPages = reviewTotalPages();
            if (currentPage < totalPages - 1) currentPage++;
            else currentPage = 0;
          }
          updateDisplay();
        }
        stopTapPending = false;
      }
    }
  }

  // ================= Normal mode state machine =================
  if (startState == STATE_DELAY) {
    if (now - delayStartTime >= delayDuration) {
      startState = STATE_RUNNING;
      runStartTime = now;
      blinkTime = now;

      buzzerActive = true;
      buzzerStartTime = now;
      analogWrite(BUZZER_PIN, buzzerVolume);

      shotCount = 0;
      currentPage = 0;
      updateDisplay();
    }
    return;
  }

  if (startState == STATE_RUNNING) {
    int peak = measurePeakToPeak();
    bool loud = (peak > soundThresholdSetting);

    if (now - blinkTime >= 500) {
      blinkTime = now;
      ledOn = !ledOn;
      digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
    }

    if (loud && !lastLoudState && shotCount < MAX_SHOTS) {
      if (now - lastEventTime >= cooldownMs) {
        // v1.7.5: measure the gap with the 32-bit clock and store the stamp as
        // previous stamp + gap, capping any gap over 65.5 s at GAP_CAP_MS so it
        // can't wrap. (lastEventTime is the previous shot; runStartTime the beep.)
        unsigned long prevMs = (shotCount == 0) ? runStartTime : lastEventTime;
        uint16_t prevStamp = (shotCount == 0) ? (uint16_t)runStartTime : shotTimes[shotCount - 1];
        unsigned long gap = now - prevMs;
        if (gap > GAP_CAP_MS) gap = GAP_CAP_MS;
        shotTimes[shotCount] = (uint16_t)(prevStamp + gap);
        shotCount++;
        lastEventTime = now;

        currentPage = livePageForLatestShot();
        updateDisplay();
      }
    }

    lastLoudState = loud;

    if (buzzerActive && (now - buzzerStartTime >= 500)) {
      buzzerActive = false;
      analogWrite(BUZZER_PIN, 0);
    }
  }

  maybeBlinkBattery(now);
  cornerGaugeRefresh(now);
}
