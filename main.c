#include "main.h"

void main(void)
{
    system_config_clock_init();
    watchdog_enable(WATCHDOG_TIMEOUT_DURATION_MS, WATCHDOG_PAUSE_ON_DEBUG);
    user_application_init();

    while (1)
    {
        watchdog_update();
        user_application_run();
    }
}