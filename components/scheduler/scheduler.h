#ifndef SCHEDULER_H
#define SCHEDULER_H

/* STANDARD INCLUDES */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* LOCAL INCLUDES */
#include "common/system_utils.h"

/* MACRO DEFINITIONS */
#define SCHEDULER_MAX_QUEUE_SIZE                            (50)
#define SCHEDULER_MAX_DATA_BUFFER_SIZE                      (10)

/* CALLBACK HANDLER TYPEDEFS  */
typedef void (*scheduler_event_handler_t)(void *p_event_data, uint8_t event_data_size);

/* ENUMERATIONS */
typedef enum
{
    SCHEDULER_OPERATION_STATUS_SUCCESS = 0,
    SCHEDULER_OPERATION_STATUS_INVALID_LENGTH,
    SCHEDULER_OPERATION_STATUS_QUEUE_FULL,
} scheduler_operation_status_e;

/* STRUCTURES */
typedef struct
{
    scheduler_event_handler_t event_handler[SCHEDULER_MAX_QUEUE_SIZE];
    uint8_t data_buffer[SCHEDULER_MAX_QUEUE_SIZE][SCHEDULER_MAX_DATA_BUFFER_SIZE];
    uint8_t data_length[SCHEDULER_MAX_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    bool is_empty;
} scheduler_params_t;

/* FUNCTION PROTOTYPES */
scheduler_operation_status_e scheduler_init(void);
scheduler_operation_status_e scheduler_event_put(void *p_event_data, uint16_t event_data_size, scheduler_event_handler_t event_handler);
void scheduler_execute(void);

#endif /* SCHEDULER_H */