#include QMK_KEYBOARD_H

// ═══════════════════════════════════════════════════════════════════════════════
//  MACROPAD KEYMAP
// ═══════════════════════════════════════════════════════════════════════════════
//
//  Physical layout:
//
//   [   ] [   ] [   ] [ENC0] [ENC1]
//   [ TL] [ T1] [ T2] [ T3 ] [ TR ]   ← top key row
//   [ ML] [ M1] [ M2] [ M3 ] [     ]
//   [ BL] [ B1] [ B2] [ BR ] [ 2U  ]  ← 2u tall key right side
//
//  LAYOUT() argument order (matches info.json, physical left→right top→bottom):
//    LAYOUT(TL, T1, T2, T3, TR,   ML, M1, M2, M3,   2U,   BL, B1, B2, BR,   EC0, EC1)
//
//  4 LAYERS — BR key cycles through them:
//   🔵 BASE  (blue)   — Productivity
//   🩷 MEDIA (pink)   — Music & media
//   🟠 GAME  (orange) — Streaming, mic, Discord
//   🟣 CFG   (purple) — RGB config
//
//  Standalone LED (bottom-left of board) breathes current layer color.
//
// ───────────────────────────────────────────────────────────────────────────────
//  HOW TO CHANGE A KEY: replace any alias below with a keycode
//  HOW TO CHANGE A SHORTCUT: edit the #define block below
//  HOW TO CHANGE ENCODER TURNS: edit encoder_map[] at the bottom
//  Full keycode list → https://docs.qmk.fm/keycodes
// ═══════════════════════════════════════════════════════════════════════════════

// ── Layers ────────────────────────────────────────────────────────────────────
enum layers { BASE, MEDIA, GAME, CFG };

// ── Custom keycodes ───────────────────────────────────────────────────────────
enum custom_keycodes {
    CP_KEY = SAFE_RANGE,  // smart copy→paste toggle
    MIC_KEY,              // mic mute toggle (LED feedback)
    LNXT,                 // cycle to next layer
};

// ═══════════════════════════════════════════════════════════════════════════════
//  SHORTCUT ALIASES — edit here to remap globally
// ═══════════════════════════════════════════════════════════════════════════════
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
#define S_PWIN  LGUI(LSFT(KC_S))    // Win+Shift+S snip tool
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
// Discord — set matching hotkeys in Discord Settings → Keybinds
#define S_DCMUT LCTL(LSFT(KC_M))    // mic mute    (default Ctrl+Shift+M)
#define S_DCDEF LCTL(LSFT(KC_D))    // deafen      (default Ctrl+Shift+D)
// Clip / record — Xbox Game Bar (built into Windows)
#define S_CLIP  LGUI(LALT(KC_G))    // Win+Alt+G  save last 30s
#define S_RECRD LGUI(LALT(KC_R))    // Win+Alt+R  record toggle
#define S_GBAR  LGUI(KC_G)          // Win+G      open Game Bar
// OBS — assign F13-F16 in OBS Settings → Hotkeys
#define S_OBS1  KC_F13
#define S_OBS2  KC_F14
#define S_OBS3  KC_F15
#define S_OBSST KC_F16              // OBS start/stop stream


// ── State ─────────────────────────────────────────────────────────────────────
static bool clipboard_full = false;
static bool mic_muted      = false;

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
    }
    return true;
}


// ═══════════════════════════════════════════════════════════════════════════════
//  RGB LIGHTING
//
//  LED physical grid (chain indices):
//   [13][14][ 8][ 7][ 0]   top row     TL T1 T2 T3 TR
//   [12][15][ 9][ 6][ 1]   middle row  ML M1 M2 M3 2U(top)
//   [ 5][11][10][ 3][ 2]   bottom row  BL B1 B2 BR 2U(bot)
//   [ 4] = standalone status LED — layer indicator
//
//  Special indicators:
//    LED 4  — breathes current layer color (standalone, always visible)
//    LED 7  — T3 = CP_KEY: breathes green/teal when clipboard has content
//    LED 13 — TL = MIC_KEY (GAME): breathes red=muted, solid green=live
//    LED 1+2 — both halves of 2u key always match
// ═══════════════════════════════════════════════════════════════════════════════

static inline uint8_t breath_val(void) {
    // Smooth 0-255 sine-like breath tied to timer
    uint16_t t   = timer_read() & 0x1FFF;           // 0-8191
    uint16_t tri = (t < 4096) ? t : 8192 - t;       // triangle 0-4095
    return (uint8_t)(tri >> 4);                      // 0-255
}

#define DIM(v, pct)   ((uint8_t)((uint16_t)(v) * (pct) / 100))
#define L(i,r,g,b)    rgb_matrix_set_color((i),(r),(g),(b))
#define L2U(r,g,b)    do { L(1,r,g,b); L(2,r,g,b); } while(0)

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t br    = breath_val();

    // ── BASE: productivity color coding ───────────────────────────────────────
    if (layer == BASE) {
        L(13, 255, 120,   0);   // TL  Undo       amber
        L(14, 200, 180,   0);   // T1  Redo       gold
        L( 8, 255,  50,  20);   // T2  Cut        coral
        // T3 (LED7) = CP_KEY — handled below
        L( 0,  30, 160, 255);   // TR  Save       electric blue
        L(12,   0, 200, 180);   // ML  Find       teal
        L(15,   0, 160, 160);   // M1  Find+Repl  dark teal
        L( 9,  80, 220, 100);   // M2  Sel All    mint
        L( 6, 160,  60, 255);   // M3  Snip       violet
        L( 5,   0, 220,  80);   // BL  New Tab    green
        L(11, 255,  60,   0);   // B1  Close Tab  orange
        L(10,  60, 120, 220);   // B2  Alt+Tab    periwinkle
        // BR (LED3) = LNXT — handled below
        L2U(200, 200, 200);     // 2U  Enter      soft white

    // ── MEDIA: purples & pinks ────────────────────────────────────────────────
    } else if (layer == MEDIA) {
        L(13, 180,   0, 255);   // TL  Prev
        L(14, 255,  20, 200);   // T1  Play/Pause
        L( 8, 180,   0, 255);   // T2  Next
        L( 7, 120,   0, 180);   // T3  Stop
        L( 0, 255,   0, 120);   // TR  Mute
        L(12, 200,   0,  80);   // ML  Vol-
        L(15, 255,  80, 140);   // M1  Vol+
        L( 9, 200, 200,   0);   // M2  Bri-
        L( 6, 255, 255,  40);   // M3  Bri+
        L( 5,  50, 100, 200);   // BL  Win+Tab
        L(11,  40,  80, 160);   // B1  Desktop
        L(10, 100,  40, 160);   // B2  Lock
        L2U(255,  20, 200);     // 2U  Play/Pause

    // ── GAME: streaming & comms ───────────────────────────────────────────────
    } else if (layer == GAME) {
        // TL (LED13) = MIC_KEY — handled below
        L(14,  66,  82, 234);   // T1  Discord deafen  Discord blue
        L( 8, 255,  30,  30);   // T2  Save clip       record red
        L( 7, 255,  60,   0);   // T3  Screenshot      orange
        L( 0,   0, 180,  60);   // TR  OBS stream      OBS green
        L(12,   0, 160,  50);   // ML  OBS scene 1     green
        L(15,   0, 140,  40);   // M1  OBS scene 2
        L( 9,   0, 120,  35);   // M2  OBS scene 3
        L( 6, 200,  20,  20);   // M3  OBS record      record red
        L( 5, 140,   0, 200);   // BL  Prev            purple
        L(11, 180,   0, 240);   // B1  Play/Pause
        L(10, 140,   0, 200);   // B2  Next
        L2U(255, 0, 0);         // 2U  Speaker mute    bold red

        // MIC key (TL, LED13): breathes red=muted, solid green=live
        if (13 >= led_min && 13 < led_max) {
            if (mic_muted) {
                uint8_t pulse = DIM(60, 40) + DIM(br, 60);
                L(13, pulse, 0, 0);
            } else {
                L(13, 0, 255, 80);
            }
        }

    // ── CFG: per-key rainbow ──────────────────────────────────────────────────
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
        L2U(200, 200, 200);     // 2U  BOOT — white
    }

    // ── LAYER CYCLE KEY (BR, LED3) — previews next layer color ───────────────
    if (3 >= led_min && 3 < led_max) {
        uint8_t next = (layer + 1) % 4;
        switch (next) {
            case BASE:  L(3,   0, 100, 220); break;
            case MEDIA: L(3, 200,   0, 160); break;
            case GAME:  L(3, 220,  60,   0); break;
            case CFG:   L(3, 120,   0, 220); break;
        }
    }

    // ── COPY/PASTE KEY (T3, LED7) — breathes green when clipboard full ────────
    if (7 >= led_min && 7 < led_max && layer != GAME) {
        if (clipboard_full) {
            uint8_t g = DIM(100, 40) + DIM(br, 60);
            L(7, 0, g, DIM(g, 25));
        }
    }

    // ── STANDALONE STATUS LED (LED4) — breathes current layer color ───────────
    if (4 >= led_min && 4 < led_max) {
        uint8_t p = DIM(160, 30) + DIM(br, 70);
        switch (layer) {
            case BASE:  L(4,   0, DIM(p,50), p         ); break;  // blue
            case MEDIA: L(4, p,   0,         DIM(p,70) ); break;  // pink
            case GAME:  L(4, p,   DIM(p,25), 0         ); break;  // orange
            case CFG:   L(4, DIM(p,55), 0,  p          ); break;  // purple
        }
    }

    return false;
}


// ═══════════════════════════════════════════════════════════════════════════════
//  KEYMAPS
//  Argument order: TL, T1, T2, T3, TR,  ML, M1, M2, M3,  2U,  BL, B1, B2, BR,  EC0, EC1
// ═══════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

// ─── BASE: Productivity ───────────────────────────────────────────────────────
//   [Undo] [Redo] [ Cut] [Copy/Paste●] [Save]
//   [Find] [F+Rep] [SelAll] [Snip]
//                                       [Enter]
//   [NwTab] [ClTab] [AltTab] [▶CYCLE]
//
    [BASE] = LAYOUT(
    //  TL       T1       T2       T3       TR
        S_UNDO,  S_REDO,  S_CUT,   CP_KEY,  S_SAVE,
    //  ML       M1       M2       M3
        S_FIND,  S_REPL,  S_SALL,  S_PWIN,
    //  2U
        KC_ENT,
    //  BL       B1       B2       BR
        S_NTAB,  S_CTAB,  S_ALTT,  LNXT,
    //  EC0      EC1
        S_MUTE,  S_PLAY
    ),

// ─── MEDIA: Music & system ────────────────────────────────────────────────────
//   [Prev] [Play] [Next] [Stop] [Mute]
//   [Vol-] [Vol+] [Bri-] [Bri+]
//                                [Play]
//   [WinTab] [Desktop] [Lock] [▶CYCLE]
//
    [MEDIA] = LAYOUT(
        S_PREV,  S_PLAY,  S_NEXT,  S_STOP,  S_MUTE,
        S_VOLD,  S_VOLU,  S_BRID,  S_BRIU,
        S_PLAY,
        S_WTAB,  S_DESK,  S_LOCK,  LNXT,
        S_MUTE,  S_PLAY
    ),

// ─── GAME: Streaming & comms ──────────────────────────────────────────────────
//   [🎙MIC] [DCDeafn] [Clip] [Scrnsht] [OBSStream]
//   [OBSSc1] [OBSSc2] [OBSSc3] [OBSRec]
//                                        [SpkrMute]
//   [Prev] [Play] [Next] [▶CYCLE]
//
    [GAME] = LAYOUT(
        MIC_KEY, S_DCDEF, S_CLIP,  S_PWIN,  S_OBSST,
        S_OBS1,  S_OBS2,  S_OBS3,  S_RECRD,
        S_MUTE,
        S_PREV,  S_PLAY,  S_NEXT,  LNXT,
        MIC_KEY, S_DCDEF
    ),

// ─── CFG: RGB & config ────────────────────────────────────────────────────────
//   [RGBtog] [Mode+] [Hue+] [Hue-] [Mode-]
//   [Sat+] [Sat-] [Bri+] [Bri-]
//                            [BOOT]
//   [Spd+] [Spd-] [    ] [▶CYCLE]
//
    [CFG] = LAYOUT(
        RM_TOGG, RM_NEXT, RM_HUEU, RM_HUED, RM_PREV,
        RM_SATU, RM_SATD, RM_VALU, RM_VALD,
        QK_BOOT,
        RM_SPDU, RM_SPDD, KC_NO,   LNXT,
        RM_HUED, RM_HUEU
    ),
};


// ═══════════════════════════════════════════════════════════════════════════════
//  ENCODER MAP   ENCODER_CCW_CW(turn-left, turn-right)
//  ENC0 = left knob    ENC1 = right knob
// ═══════════════════════════════════════════════════════════════════════════════
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
//              ENC0 (left)                        ENC1 (right)
    [BASE]  = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(MS_WHLD, MS_WHLU) },
    [MEDIA] = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(S_PREV,  S_NEXT)  },
    [GAME]  = { ENCODER_CCW_CW(S_VOLD, S_VOLU),   ENCODER_CCW_CW(S_VOLD,  S_VOLU)  },
    [CFG]   = { ENCODER_CCW_CW(RM_HUED, RM_HUEU), ENCODER_CCW_CW(RM_VALD, RM_VALU) },
};
