#ifndef USER_APPLICATION_H
#define USER_APPLICATION_H

/* STANDARD INCLUDES */
#include <stdint.h>

/* MACRO DEFINITIONS */
#define USER_APPLICATION_STEPPER_MOTOR_STEP_SPEED           (2000)

/* LOCAL INCLUDES */
#include "components/scheduler/scheduler.h"
#include "components/ipc/ipc.h"
#include "drivers/stepper_motor/stepper_motor_drv.h"
#include "drivers/encoder/encoder_drv.h"
#include "application/user_application_messages.h"

/* ENUMERATIONS */
typedef enum
{
    USER_APPLICATION_STATE_MACHINE_STATE_CALIBRATION = 0,
    USER_APPLICATION_STATE_MACHINE_STATE_READY,
    USER_APPLICATION_STATE_MACHINE_STATE_EXECUTION
} user_application_state_machine_state_e;

typedef enum
{
    USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL = 0,
    USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_PROCESS_CMD_REQUEST,
    USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_STEP_ACTIVITY,
    USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_HOME_ACTIVITY,
} user_application_state_machine_event_type_e;

/* STRUCTURES */
typedef struct __attribute__((packed))
{
    user_application_messages_cmd_type_e saved_cmd_type;
    user_application_messages_resp_status_e resp_status;
    uint8_t position_requested;
} user_application_state_machine_params_t;

typedef struct __attribute__((packed))
{
    user_application_messages_cmd_set_stepper_motor_angle_t cmd_set_stepper_motor_angle;
} user_application_state_machine_process_cmd_request_event_data_u;

typedef union
{
    user_application_state_machine_process_cmd_request_event_data_u process_cmd_request_event_data;
} user_application_state_machine_event_specific_data_u;

typedef struct __attribute__((packed))
{
    user_application_state_machine_event_type_e type;
    user_application_state_machine_event_specific_data_u event_specific_data;
} user_application_state_machine_event_data_t;

/* FUNCTION PROTOTYPES */
void user_application_init(void);
void user_application_run(void);

#endif /* USER_APPLICATION_H */
