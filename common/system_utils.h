#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

/* STANDARD INCLUDES */
#include <stdint.h>

/* LOCAL INCLUDES */
#include "hardware/sync.h"

/* FUNCTION PROTOTYPES */
void system_utils_enable_interrupts(void);
void system_utils_disable_interrupts(void);

#endif /* SYSTEM_UTILS_H */
