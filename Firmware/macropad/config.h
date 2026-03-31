#pragma once

// ── Matrix ────────────────────────────────────────────────────────────────────
#define MATRIX_ROWS 4
#define MATRIX_COLS 4
#define MATRIX_ROW_PINS { GP3, GP4, GP2, GP1 }

// ── 74HC165 shift register ────────────────────────────────────────────────────
//   GP27=~PL(latch)  GP28=CP(clock)  GP26=Q7(data)
//   Bits: [7]=ENC0-B [6]=ENC0-A [5]=ENC1-B [4]=ENC1-A [3]=COL3 [2]=COL2 [1]=COL1 [0]=COL0
#define SHIFT_REG_PL   GP27
#define SHIFT_REG_CLK  GP28
#define SHIFT_REG_DATA GP26

// ── Encoders ──────────────────────────────────────────────────────────────────
#define NUM_ENCODERS       2
#define ENCODER_RESOLUTION 4

// ── Timings ───────────────────────────────────────────────────────────────────
#define DEBOUNCE     5
#define TAPPING_TERM 200

// ── I2C (for OLED — connects via J1: GP6=SDA, GP7=SCL) ───────────────────────
#define I2C_DRIVER   I2CD1
#define I2C1_SDA_PIN GP6
#define I2C1_SCL_PIN GP7

// ── OLED (SSD1306 128x32) ─────────────────────────────────────────────────────
#define OLED_DISPLAY_128X32
#define OLED_TIMEOUT     60000   // turn off after 60s of no activity (ms)
#define OLED_BRIGHTNESS  200
