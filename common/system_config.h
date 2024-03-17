#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/* STANDARD INCLUDES */
#include <stdint.h>

/* LOCAL INCLUDES */
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "common/system_utils.h"

/* MACRO DEFINITIONS */
/**
 * Note: The following CPU core clock frequency has been confirmed to be achievable using the provided vcocalc.py
 * Here are the settings to be setup
 * Achieved: 133 MHz
 * REFDIV: 1
 * FBDIV: 133 (VCO = 1596.0 MHz)`
 * PD1: 6
 * PD2: 2
*/
#define SYSTEM_CONFIG_CPU_CORE_CLOCK_FREQ_KHZ               (133000)

/* FUNCTION PROTOTYPES */
void system_config_clock_init(void);

#endif /* SYSTEM_CONFIG_H */
