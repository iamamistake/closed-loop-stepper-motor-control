#ifndef MAIN_H
#define MAIN_H

/* LOCAL INCLUDES */
#include "hardware/watchdog.h"
#include "common/system_config.h"
#include "application/user_application.h"

/* MACRO DEFINITIONS */
#define WATCHDOG_TIMEOUT_DURATION_MS                        (1000)
#define WATCHDOG_PAUSE_ON_DEBUG                             (1)

/* FUNCTION PROTOTYPES */
void main(void);

#endif /* MAIN_H */
