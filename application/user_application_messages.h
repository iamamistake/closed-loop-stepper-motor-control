#ifndef USER_APPLICATION_MESSAGES_H
#define USER_APPLICATION_MESSAGES_H

/* STANDARD INCLUDES */
#include <stdint.h>

/* ENUMERATIONS */
typedef enum
{
    USER_APPLICATION_MESSAGES_CMD_TYPE_SET_STEPPER_MOTOR_ANGLE = 0,
} user_application_messages_cmd_type_e;

typedef enum
{
    USER_APPLICATION_MESSAGES_RESP_TYPE_SET_STEPPER_MOTOR_ANGLE = 0,
} user_application_messages_resp_type_e;

typedef enum
{
    USER_APPLICATION_MESSAGES_RESP_STATUS_OK = 0,
    USER_APPLICATION_MESSAGES_RESP_STATUS_INVALID_ANGLE_REQUESTED
} user_application_messages_resp_status_e;

/* STRUCTURES */
// COMMAND MESSAGE DATA FRAME FORMATS
typedef struct __attribute__((packed))
{
    user_application_messages_cmd_type_e cmd_type;
} user_application_messages_cmd_generic_frame_t;

typedef struct __attribute__((packed))
{
    user_application_messages_cmd_type_e cmd_type;
    uint16_t angle_requested;
} user_application_messages_cmd_set_stepper_motor_angle_t;

// RESPONSE MESSAGE DATA FRAME FORMATS
typedef struct __attribute__((packed))
{
    user_application_messages_resp_type_e resp_type;
    user_application_messages_resp_status_e resp_status;
    uint16_t angle_achieved;
} user_application_messages_resp_set_stepper_motor_angle_t;

/* INLINE FUNCTIONS */
static inline user_application_messages_cmd_type_e user_application_messages_get_cmd_type(void *p_message)
{
    return (((user_application_messages_cmd_generic_frame_t *)(p_message))->cmd_type);
}

#endif /* USER_APPLICATION_MESSAGES_H */
