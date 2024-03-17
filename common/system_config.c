#include "common/system_config.h"

void system_config_clock_init(void)
{
    system_utils_disable_interrupts();

    set_sys_clock_khz(SYSTEM_CONFIG_CPU_CORE_CLOCK_FREQ_KHZ, true);

    system_utils_enable_interrupts();
}
