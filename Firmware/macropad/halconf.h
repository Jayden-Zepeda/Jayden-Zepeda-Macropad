#pragma once
#define HAL_USE_SPI  FALSE
#define HAL_USE_I2C  TRUE     // needed for OLED
#define HAL_USE_UART FALSE
#include_next <halconf.h>
