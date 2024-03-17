#include "components/scheduler/scheduler.h"

static scheduler_params_t scheduler_params = {.head = 0,
                                              .tail = 0,
                                              .is_empty = true};

scheduler_operation_status_e scheduler_init(void)
{
    scheduler_params.head = 0;
    scheduler_params.tail = 0;
    scheduler_params.is_empty = true;

    return SCHEDULER_OPERATION_STATUS_SUCCESS;
}

scheduler_operation_status_e scheduler_event_put(void *p_event_data, uint16_t event_data_size, scheduler_event_handler_t event_handler)
{
    if (event_data_size <= SCHEDULER_MAX_DATA_BUFFER_SIZE)
    {
        if (scheduler_params.is_empty)
        {
        	system_utils_disable_interrupts();

            if (p_event_data && event_data_size)
            {
                memcpy(*(scheduler_params.data_buffer + scheduler_params.head), p_event_data, event_data_size);
            }
            scheduler_params.event_handler[scheduler_params.head] = event_handler;
            scheduler_params.data_length[scheduler_params.head] = event_data_size;
            scheduler_params.head++;

            if (scheduler_params.head == SCHEDULER_MAX_QUEUE_SIZE)
            {
                scheduler_params.head = 0;
            }

            if (scheduler_params.head == scheduler_params.tail)
            {
                scheduler_params.is_empty = false;
            }

            system_utils_enable_interrupts();

            return SCHEDULER_OPERATION_STATUS_SUCCESS;
        }
        else
        {
            return SCHEDULER_OPERATION_STATUS_QUEUE_FULL;
        }
    }
    else
    {
        return SCHEDULER_OPERATION_STATUS_INVALID_LENGTH;
    }
}

void scheduler_execute(void)
{
    while ((scheduler_params.head != scheduler_params.tail) || (scheduler_params.is_empty == false))
    {
        (scheduler_params.event_handler[scheduler_params.tail])(scheduler_params.data_buffer[scheduler_params.tail], scheduler_params.data_length[scheduler_params.tail]);
        scheduler_params.tail++;

        if (scheduler_params.tail == SCHEDULER_MAX_QUEUE_SIZE)
        {
            scheduler_params.tail = 0;
        }

        scheduler_params.is_empty = true;
    }
}
