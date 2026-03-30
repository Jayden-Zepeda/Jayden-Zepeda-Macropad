#include "macropad.h"
#include "rgb_matrix.h"

/*
 * LED chain (D1=idx0 ... D16=idx15) physical positions:
 *
 *   [13][14][ 8][ 7][ 0]   top row     TL T1 T2 T3 TR
 *   [12][15][ 9][ 6][ 1]   middle row  ML M1 M2 M3 2U(top)
 *   [ 5][11][10][ 3][ 2]   bottom row  BL B1 B2 BR 2U(bot)
 *   [ 4] = standalone status LED (far bottom-left of board)
 *
 * matrix_co[row][col] = LED index. NO_LED for positions with no LED.
 */
led_config_t g_led_config = {
    {
        //        C0     C1      C2     C3
        /* R0 */ {  0,    7,     8,    14  },   // TR  T3  T2  T1
        /* R1 */ {  6,    9,    15,    12  },   // M3  M2  M1  ML
        /* R2 */ {  3,   10,    11,     5  },   // BR  B2  B1  BL
        /* R3 */ {  1,  NO_LED, NO_LED, 13 },   // 2U  ec0 ec1 TL
    },
    {
        // Physical x,y normalized to 0-224 / 0-64
        // PCB key x: 187-265mm (78mm span)  y: 45-100mm (55mm span)
        // x_n = (pcb_x-187)/78*224   y_n = (pcb_y-45)/55*64
        { 218,  0 },  // 0  D1  TR   pcb(263,45)
        { 218, 25 },  // 1  D2  2U top   pcb(263,67)
        { 220, 64 },  // 2  D3  2U bot   pcb(264,100)
        { 164, 45 },  // 3  D4  BR   pcb(244,84)
        {   0, 57 },  // 4  D5  standalone status LED  pcb(152,94)
        {   0, 45 },  // 5  D6  BL   pcb(187,84)
        { 164, 23 },  // 6  D7  M3   pcb(244,65)
        { 164,  0 },  // 7  D8  T3   pcb(244,45)
        { 109,  0 },  // 8  D9  T2   pcb(225,45)
        { 109, 23 },  // 9  D10 M2   pcb(225,65)
        { 109, 45 },  // 10 D11 B2   pcb(225,84)
        {  55, 45 },  // 11 D12 B1   pcb(206,84)
        {   0, 23 },  // 12 D13 ML   pcb(187,65)
        {   0,  0 },  // 13 D14 TL   pcb(187,45)
        {  55,  0 },  // 14 D15 T1   pcb(206,45)
        {  55, 23 },  // 15 D16 M1   pcb(206,65)
    },
    {
        LED_FLAG_KEYLIGHT,   // 0  TR
        LED_FLAG_KEYLIGHT,   // 1  2U top
        LED_FLAG_KEYLIGHT,   // 2  2U bottom
        LED_FLAG_KEYLIGHT,   // 3  BR
        LED_FLAG_INDICATOR,  // 4  standalone status (not under a key)
        LED_FLAG_KEYLIGHT,   // 5  BL
        LED_FLAG_KEYLIGHT,   // 6  M3
        LED_FLAG_KEYLIGHT,   // 7  T3
        LED_FLAG_KEYLIGHT,   // 8  T2
        LED_FLAG_KEYLIGHT,   // 9  M2
        LED_FLAG_KEYLIGHT,   // 10 B2
        LED_FLAG_KEYLIGHT,   // 11 B1
        LED_FLAG_KEYLIGHT,   // 12 ML
        LED_FLAG_KEYLIGHT,   // 13 TL
        LED_FLAG_KEYLIGHT,   // 14 T1
        LED_FLAG_KEYLIGHT,   // 15 M1
    }
};
