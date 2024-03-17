#include "common/system_utils.h"

static uint32_t system_utils_save_interrupt_status = 0;

void system_utils_enable_interrupts(void)
{
    restore_interrupts(system_utils_save_interrupt_status);
}

void system_utils_disable_interrupts(void)
{
    system_utils_save_interrupt_status =  save_and_disable_interrupts();
}
