#pragma once
#include "quantum.h"

/*
 * The LAYOUT macro is auto-generated from info.json.
 * Physical argument order (matches info.json layout array order):
 *
 *   LAYOUT(
 *     TL,  T1,  T2,  T3,  TR,     <- top key row (left to right)
 *     ML,  M1,  M2,  M3,          <- middle row
 *     2U,                          <- 2u tall key (right side spans rows 2+3)
 *     BL,  B1,  B2,  BR,          <- bottom row
 *     EC0, EC1                     <- encoder click buttons
 *   )
 *
 * Physical layout:
 *   [   ] [   ] [   ] [ENC0] [ENC1]
 *   [ TL] [ T1] [ T2] [ T3 ] [ TR ]   <- top row
 *   [ ML] [ M1] [ M2] [ M3 ] [     ]
 *   [ BL] [ B1] [ B2] [ BR ] [ 2U  ]  <- 2u key right side
 *
 * LED grid (chain indices D1=0 ... D16=15):
 *   [13][14][ 8][ 7][ 0]   top row
 *   [12][15][ 9][ 6][ 1]   middle row  (1 = 2u top half)
 *   [ 5][11][10][ 3][ 2]   bottom row  (2 = 2u bottom half)
 *   [ 4] = standalone status LED (bottom-left of board, layer indicator)
 */
