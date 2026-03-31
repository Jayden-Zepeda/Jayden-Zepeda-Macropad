#pragma once
#include_next <mcuconf.h>

// Enable I2C1 on the RP2040 (used for OLED via J1 connector: GP6=SDA, GP7=SCL)
#undef  RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C1 TRUE
