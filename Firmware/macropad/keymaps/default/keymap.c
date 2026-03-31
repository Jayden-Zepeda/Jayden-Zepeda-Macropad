#include QMK_KEYBOARD_H

// ═══════════════════════════════════════════════════════════════════════════════
//  MACROPAD KEYMAP + OLED
// ═══════════════════════════════════════════════════════════════════════════════
//
//  OLED (128x32, connects to J1 on the board via GP6=SDA / GP7=SCL):
//
//   ┌─────────────────────────────────────────┐
//   │ PRODUCTIVITY            52 wpm           │  layer name + WPM
//   │ VOL  [████████░░]  80%  ♪               │  volume bar + speaker icon
//   │ MIC  [ LIVE  ]   CLIP  [READY]          │  mic & clipboard status
//   │ Undo  Redo   Cut   Copy/Paste            │  key hints for current layer
//   └─────────────────────────────────────────┘
//
//  HOW TO CUSTOMIZE THE OLED
//  ─────────────────────────
//  • Change what shows on each line in oled_task_user() below.
//  • Each line uses oled_write() / oled_write_P() / oled_write_ln_P().
//  • oled_set_cursor(col, row) moves position (col=0-20, row=0-3).
//  • Use PSTR("text") for string literals to save RAM.
//  • Volume bar width = VOL_BAR_WIDTH (default 10 chars). Change freely.
//  • Starting volume = VOL_START (0-100). Macropad tracks from here.
//
// ═══════════════════════════════════════════════════════════════════════════════

// ── Layers ────────────────────────────────────────────────────────────────────
enum layers { BASE, MEDIA, GAME, CFG };

// ── Custom keycodes ───────────────────────────────────────────────────────────
enum custom_keycodes {
    CP_KEY = SAFE_RANGE,
    MIC_KEY,
    LNXT,
};

// ── Shortcut aliases ──────────────────────────────────────────────────────────
#define S_UNDO  LCTL(KC_Z)
#define S_REDO  LCTL(KC_Y)
#define S_CUT   LCTL(KC_X)
#define S_SAVE  LCTL(KC_S)
#define S_SALL  LCTL(KC_A)
#define S_FIND  LCTL(KC_F)
#define S_REPL  LCTL(KC_H)
#define S_NTAB  LCTL(KC_T)
#define S_CTAB  LCTL(KC_W)
#define S_ALTT  LALT(KC_TAB)
#define S_PWIN  LGUI(LSFT(KC_S))
#define S_LOCK  LGUI(KC_L)
#define S_DESK  LGUI(KC_D)
#define S_WTAB  LGUI(KC_TAB)
#define S_TASK  LCTL(LSFT(KC_ESC))
#define S_PREV  KC_MPRV
#define S_PLAY  KC_MPLY
#define S_NEXT  KC_MNXT
#define S_STOP  KC_MSTP
#define S_MUTE  KC_MUTE
#define S_VOLU  KC_VOLU
#define S_VOLD  KC_VOLD
#define S_BRIU  KC_BRIU
#define S_BRID  KC_BRID
#define S_DCMUT LCTL(LSFT(KC_M))
#define S_DCDEF LCTL(LSFT(KC_D))
#define S_CLIP  LGUI(LALT(KC_G))
#define S_RECRD LGUI(LALT(KC_R))
#define S_GBAR  LGUI(KC_G)
#define S_OBS1  KC_F13
#define S_OBS2  KC_F14
#define S_OBS3  KC_F15
#define S_OBSST KC_F16


// ═══════════════════════════════════════════════════════════════════════════════
//  STATE
// ═══════════════════════════════════════════════════════════════════════════════

static bool    clipboard_full = false;
static bool    mic_muted      = false;
static bool    spk_muted      = false;   // speaker mute state
static bool    recording      = false;   // OBS/Game Bar recording state

// Volume is tracked locally starting from VOL_START.
// Every volume key press adjusts by VOL_STEP.
// ↓ change VOL_START if your system volume is different when you first plug in
#define VOL_START  50    // assumed starting volume (0-100)
#define VOL_STEP    2    // how much each key press changes volume
#define VOL_BAR_WIDTH 10 // number of characters in the volume bar

static int8_t  vol_level = VOL_START;   // 0-100


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {

        case CP_KEY:
            if (record->event.pressed) {
                if (!clipboard_full) {
                    tap_code16(LCTL(KC_C));
                    clipboard_full = true;
                } else {
                    tap_code16(LCTL(KC_V));
                    clipboard_full = false;
                }
            }
            return false;

        case MIC_KEY:
            if (record->event.pressed) {
                tap_code16(S_DCMUT);
                mic_muted = !mic_muted;
            }
            return false;

        case LNXT:
            if (record->event.pressed) {
                uint8_t next = (get_highest_layer(layer_state) + 1) % 4;
                layer_clear();
                if (next != BASE) layer_on(next);
            }
            return false;

        // Track volume key presses to update the OLED bar
        case S_VOLU:
            if (record->event.pressed) {
                vol_level += VOL_STEP;
                if (vol_level > 100) vol_level = 100;
                spk_muted = false;
            }
            return true;   // still send the key

        case S_VOLD:
            if (record->event.pressed) {
                vol_level -= VOL_STEP;
                if (vol_level < 0) vol_level = 0;
                spk_muted = false;
            }
            return true;

        case S_MUTE:
            if (record->event.pressed) spk_muted = !spk_muted;
            return true;

        // Track OBS/Game Bar record state
        case S_RECRD:
            if (record->event.pressed) recording = !recording;
            return true;
    }
    return true;
}


// ═══════════════════════════════════════════════════════════════════════════════
//  OLED
// ═══════════════════════════════════════════════════════════════════════════════

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    // Change to OLED_ROTATION_180 if your OLED is upside down
    return OLED_ROTATION_0;
}

// Draw a horizontal bar — filled_chars out of total_chars
static void draw_bar(uint8_t filled, uint8_t total) {
    oled_write_char('[', false);
    for (uint8_t i = 0; i < total; i++) {
        // Use block char (0xFF = full block in most SSD1306 fonts)
        oled_write_char(i < filled ? 0xFF : 0xBB, false);
    }
    oled_write_char(']', false);
}

// Write a right-aligned number in a fixed-width field (width chars)
static void write_num(int8_t val, uint8_t width) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%*d", width, val);
    oled_write(buf, false);
}

bool oled_task_user(void) {
    uint8_t layer = get_highest_layer(layer_state);

    // ── LINE 0: Layer name + WPM ──────────────────────────────────────────────
    oled_set_cursor(0, 0);
    switch (layer) {
        case BASE:  oled_write_P(PSTR("PRODUCTIVITY    "), false); break;
        case MEDIA: oled_write_P(PSTR("MEDIA           "), false); break;
        case GAME:  oled_write_P(PSTR("STREAM/COMMS    "), false); break;
        case CFG:   oled_write_P(PSTR("RGB CONFIG      "), false); break;
    }
    // WPM right-aligned in remaining space
    uint8_t wpm = get_current_wpm();
    char wpm_buf[8];
    snprintf(wpm_buf, sizeof(wpm_buf), "%3dwpm", wpm);
    oled_write(wpm_buf, false);

    // ── LINE 1: Volume bar ────────────────────────────────────────────────────
    oled_set_cursor(0, 1);
    if (spk_muted) {
        oled_write_P(PSTR("VOL [MUTED          ]   "), false);
    } else {
        oled_write_P(PSTR("VOL "), false);
        uint8_t filled = (uint8_t)((vol_level * VOL_BAR_WIDTH) / 100);
        draw_bar(filled, VOL_BAR_WIDTH);
        write_num(vol_level, 3);
        oled_write_char('%', false);
        // Speaker icon — solid if loud, hollow if quiet
        oled_write_char(vol_level > 50 ? 0x0E : 0x0F, false);  // custom chars
    }

    // ── LINE 2: Mic + clipboard + recording status ────────────────────────────
    oled_set_cursor(0, 2);

    // Mic status
    if (mic_muted) {
        oled_write_P(PSTR("MIC[MUTE] "), false);
    } else {
        oled_write_P(PSTR("MIC[LIVE] "), false);
    }

    // Clipboard
    if (clipboard_full) {
        oled_write_P(PSTR("CLIP[READY] "), false);
    } else {
        oled_write_P(PSTR("CLIP[-----] "), false);
    }

    // ── LINE 3: Key hints — changes per layer ─────────────────────────────────
    // ↓ Change these strings to whatever is most useful to you
    oled_set_cursor(0, 3);
    switch (layer) {
        case BASE:
            oled_write_P(PSTR("Undo Redo Cut  Paste"), false);
            break;
        case MEDIA:
            oled_write_P(PSTR("|<   Play  >|  Mute "), false);
            break;
        case GAME:
            oled_write_P(PSTR("Mic  Deaf  Clip  OBS"), false);
            // Override right side with recording status
            oled_set_cursor(14, 3);
            if (recording) {
                oled_write_P(PSTR("[REC]"), false);
            } else {
                oled_write_P(PSTR("[   ]"), false);
            }
            break;
        case CFG:
            oled_write_P(PSTR("Hue  Sat  Val  Speed"), false);
            break;
    }

    return false;
}


// ═══════════════════════════════════════════════════════════════════════════════
//  RGB LIGHTING (unchanged from working version)
// ═══════════════════════════════════════════════════════════════════════════════

static inline uint8_t breath_val(void) {
    uint16_t t   = timer_read() & 0x1FFF;
    uint16_t tri = (t < 4096) ? t : 8192 - t;
    return (uint8_t)(tri >> 4);
}

#define DIM(v, pct)   ((uint8_t)((uint16_t)(v) * (pct) / 100))
#define L(i,r,g,b)    rgb_matrix_set_color((i),(r),(g),(b))
#define L2U(r,g,b)    do { L(1,r,g,b); L(2,r,g,b); } while(0)

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t br    = breath_val();

    if (layer == BASE) {
        L(13, 255, 120,   0);
        L(14, 200, 180,   0);
        L( 8, 255,  50,  20);
        L( 0,  30, 160, 255);
        L(12,   0, 200, 180);
        L(15,   0, 160, 160);
        L( 9,  80, 220, 100);
        L( 6, 160,  60, 255);
        L( 5,   0, 220,  80);
        L(11, 255,  60,   0);
        L(10,  60, 120, 220);
        L2U(200, 200, 200);

    } else if (layer == MEDIA) {
        L(13, 180,   0, 255);
        L(14, 255,  20, 200);
        L( 8, 180,   0, 255);
        L( 7, 120,   0, 180);
        L( 0, 255,   0, 120);
        L(12, 200,   0,  80);
        L(15, 255,  80, 140);
        L( 9, 200, 200,   0);
        L( 6, 255, 255,  40);
        L( 5,  50, 100, 200);
        L(11,  40,  80, 160);
        L(10, 100,  40, 160);
        L2U(255,  20, 200);

    } else if (layer == GAME) {
        L(14,  66,  82, 234);
        L( 8, 255,  30,  30);
        L( 7, 255,  60,   0);
        L( 0,   0, 180,  60);
        L(12,   0, 160,  50);
        L(15,   0, 140,  40);
        L( 9,   0, 120,  35);
        L( 6, recording ? 255 : 200, recording ? 0 : 20, 20);  // pulses red when recording
        L( 5, 140,   0, 200);
        L(11, 180,   0, 240);
        L(10, 140,   0, 200);
        L2U(255, 0, 0);

        if (13 >= led_min && 13 < led_max) {
            if (mic_muted) {
                uint8_t pulse = DIM(60, 40) + DIM(br, 60);
                L(13, pulse, 0, 0);
            } else {
                L(13, 0, 255, 80);
            }
        }

    } else if (layer == CFG) {
        const uint8_t key_leds[] = {13,14,8,7,0, 12,15,9,6, 5,11,10,3};
        for (uint8_t i = 0; i < 13; i++) {
            uint8_t idx = key_leds[i];
            if (idx < led_min || idx >= led_max) continue;
            uint8_t hue = i * 20;
            uint8_t h   = hue / 43;
            uint8_t f   = (hue % 43) * 6;
            uint8_t q   = DIM(180, 255 - f);
            uint8_t t2  = DIM(180, f);
            switch (h % 6) {
                case 0: L(idx, 180,  t2,   0); break;
                case 1: L(idx,   q, 180,   0); break;
                case 2: L(idx,   0, 180,  t2); break;
                case 3: L(idx,   0,   q, 180); break;
                case 4: L(idx,  t2,   0, 180); break;
                default:L(idx, 180,   0,   q); break;
            }
        }
        L2U(200, 200, 200);
    }

    if (3 >= led_min && 3 < led_max) {
        uint8_t next = (layer + 1) % 4;
        switch (next) {
            case BASE:  L(3,   0, 100, 220); break;
            case MEDIA: L(3, 200,   0, 160); break;
            case GAME:  L(3, 220,  60,   0); break;
            case CFG:   L(3, 120,   0, 220); break;
        }
    }

    if (7 >= led_min && 7 < led_max && layer != GAME) {
        if (clipboard_full) {
            uint8_t g = DIM(100, 40) + DIM(br, 60);
            L(7, 0, g, DIM(g, 25));
        }
    }

    if (4 >= led_min && 4 < led_max) {
        uint8_t p = DIM(160, 30) + DIM(br, 70);
        switch (layer) {
            case BASE:  L(4,   0, DIM(p,50), p         ); break;
            case MEDIA: L(4, p,   0,         DIM(p,70) ); break;
            case GAME:  L(4, p,   DIM(p,25), 0         ); break;
            case CFG:   L(4, DIM(p,55), 0,  p          ); break;
        }
    }

    return false;
}


// ═══════════════════════════════════════════════════════════════════════════════
//  KEYMAPS
// ═══════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [BASE] = LAYOUT(
        S_UNDO,  S_REDO,  S_CUT,   CP_KEY,  S_SAVE,
        S_FIND,  S_REPL,  S_SALL,  S_PWIN,
        KC_ENT,
        S_NTAB,  S_CTAB,  S_ALTT,  LNXT,
        S_MUTE,  S_PLAY
    ),

    [MEDIA] = LAYOUT(
        S_PREV,  S_PLAY,  S_NEXT,  S_STOP,  S_MUTE,
        S_VOLD,  S_VOLU,  S_BRID,  S_BRIU,
        S_PLAY,
        S_WTAB,  S_DESK,  S_LOCK,  LNXT,
        S_MUTE,  S_PLAY
    ),

    [GAME] = LAYOUT(
        MIC_KEY, S_DCDEF, S_CLIP,  S_PWIN,  S_OBSST,
        S_OBS1,  S_OBS2,  S_OBS3,  S_RECRD,
        S_MUTE,
        S_PREV,  S_PLAY,  S_NEXT,  LNXT,
        MIC_KEY, S_DCDEF
    ),

    [CFG] = LAYOUT(
        RM_TOGG, RM_NEXT, RM_HUEU, RM_HUED, RM_PREV,
        RM_SATU, RM_SATD, RM_VALU, RM_VALD,
        QK_BOOT,
        RM_SPDU, RM_SPDD, KC_NO,   LNXT,
        RM_HUED, RM_HUEU
    ),
};


// ═══════════════════════════════════════════════════════════════════════════════
//  ENCODER MAP
// ═══════════════════════════════════════════════════════════════════════════════
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [BASE]  = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(MS_WHLD, MS_WHLU) },
    [MEDIA] = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(S_PREV,  S_NEXT)  },
    [GAME]  = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(S_VOLD,  S_VOLU)  },
    [CFG]   = { ENCODER_CCW_CW(RM_HUED, RM_HUEU), ENCODER_CCW_CW(RM_VALD, RM_VALU) },
};
