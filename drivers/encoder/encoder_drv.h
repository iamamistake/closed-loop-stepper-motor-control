#ifndef ENCODER_DRV_H
#define ENCODER_DRV_H

/* STANDARD INCLUDES */
#include <stdbool.h>
#include <stdint.h>

/* LOCAL INCLUDES */
#include "hardware/gpio.h"
#include "common/system_utils.h"
#include "components/scheduler/scheduler.h"

/* MACRO DEFINITIONS */
#define ENCODER_DRV_PIN_A_ANTICLOCKWISE                     (18)
#define ENCODER_DRV_PIN_B_CLOCKWISE                         (20)
#define ENCODER_DRV_PIN_Z_HOME                              (19)

#define ENCODER_DRV_ENCODER_POSITION_RESOLUTION             (180)
#define ENCODER_DRV_ENCODER_POSITION_LOWER_BOUND            (0)
#define ENCODER_DRV_ENCODER_POSITION_HIGHER_BOUND           (ENCODER_DRV_ENCODER_POSITION_RESOLUTION - 1)

#define ENCODER_DRV_ENCODER_ANGLE_TO_POSITION(angle)        ((angle * ENCODER_DRV_ENCODER_POSITION_RESOLUTION)/360)
#define ENCODER_DRV_ENCODER_POSITION_TO_ANGLE(position)     ((position * 360)/ENCODER_DRV_ENCODER_POSITION_RESOLUTION)
#define ENCODER_DRV_IS_ENCODER_POSITION_VALID(position)     ((position >= ENCODER_DRV_ENCODER_POSITION_LOWER_BOUND) && (position <= ENCODER_DRV_ENCODER_POSITION_HIGHER_BOUND))

/* CUSTOM DATA TYPES */
typedef uint8_t encoder_drv_encoder_position_t;

/* CALLBACK HANDLER TYPEDEFS  */
typedef void (*encoder_drv_step_activity_event_handler_t)(void *p_event_data, uint8_t event_data_size);
typedef void (*encoder_drv_home_activity_event_handler_t)(void *p_event_data, uint8_t event_data_size);

/* ENUMERATIONS */
typedef enum
{
    ENCODER_DRV_OPERATION_STATUS_OK = 0,
    ENCODER_DRV_OPERATION_STATUS_INVALID_POSITION,
    ENCODER_DRV_OPERATION_STATUS_INVALID_EVENT_HANDLING
} encoder_drv_operation_status_e;

typedef enum
{
    ENCODER_DRV_EVENT_HANDLING_IMMEDIATE = 0,
    ENCODER_DRV_EVENT_HANDLING_SCHEDULED
} encoder_drv_event_handling_e;

/* STRUCTURES */
typedef struct __attribute__((packed))
{
    encoder_drv_step_activity_event_handler_t step_activity_event_handler;
    encoder_drv_home_activity_event_handler_t home_activity_event_handler;
} encoder_drv_event_handlers_t;

typedef struct __attribute__((packed))
{
    encoder_drv_event_handlers_t event_handlers;
    encoder_drv_event_handling_e event_handling;
} encoder_drv_init_params_t;

/* FUNCTION PROTOTYPES */
encoder_drv_operation_status_e encoder_drv_init(encoder_drv_init_params_t init_params);
encoder_drv_operation_status_e encoder_drv_get_encoder_position(encoder_drv_encoder_position_t *p_encoder_position);
encoder_drv_operation_status_e encoder_drv_set_encoder_position(encoder_drv_encoder_position_t *p_encoder_position);

#endif /* ENCODER_DRV_H */
