#include "application/user_application.h"

static void user_application_process_received_data_packet_handler(void *p_event_data, uint8_t event_data_size);
static void user_application_encoder_step_activity_handler(void *p_event_data, uint8_t event_data_size);
static void user_application_encoder_home_activity_handler(void *p_event_data, uint8_t event_data_size);
static void user_application_state_machine_event_handler(void *p_event_data, uint8_t event_data_size);

void user_application_init(void)
{
    ipc_init_params_t ipc_init_params = {.data_packet_received_event_handler = user_application_process_received_data_packet_handler};
    ipc_init(ipc_init_params);

    encoder_drv_init_params_t encoder_drv_init_params = {.event_handlers = {.step_activity_event_handler = user_application_encoder_step_activity_handler,
                                                                            .home_activity_event_handler = user_application_encoder_home_activity_handler},
                                                         .event_handling = ENCODER_DRV_EVENT_HANDLING_IMMEDIATE};
    encoder_drv_init(encoder_drv_init_params);

    stepper_motor_drv_init_params_t stepper_motor_drv_init_params = {.step_mode = STEPPER_MOTOR_DRV_STEP_MODE_1_16,
                                                                     .step_speed = USER_APPLICATION_STEPPER_MOTOR_STEP_SPEED};
    stepper_motor_drv_init(stepper_motor_drv_init_params);

    scheduler_init();

    user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL};
    scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
}

void user_application_run(void)
{
    scheduler_execute();
}

static void user_application_process_received_data_packet_handler(void *p_event_data, uint8_t event_data_size)
{
    ipc_data_packet_received_event_data_t *data_packet_received_event_data = (ipc_data_packet_received_event_data_t *)(p_event_data);

    if (data_packet_received_event_data->reception_status == IPC_RECEPTION_STATUS_OK)
    {
        user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_PROCESS_CMD_REQUEST,
                                                                                .event_specific_data = {.process_cmd_request_event_data = *((user_application_state_machine_process_cmd_request_event_data_u *)(data_packet_received_event_data->p_received_data))}};

        scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
    }
    else
    {
        // Do not process the received data packet since it has been possibly corrupted
    }
}

static void user_application_encoder_step_activity_handler(void *p_event_data, uint8_t event_data_size)
{
    user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_STEP_ACTIVITY};
    scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
}

static void user_application_encoder_home_activity_handler(void *p_event_data, uint8_t event_data_size)
{
    user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_HOME_ACTIVITY};
    scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
}


static void user_application_state_machine_event_handler(void *p_event_data, uint8_t event_data_size)
{
    static user_application_state_machine_state_e user_application_state_machine_state = USER_APPLICATION_STATE_MACHINE_STATE_CALIBRATION;
    static user_application_state_machine_params_t user_application_state_machine_params = {.saved_cmd_type = 0,
                                                                                            .resp_status = USER_APPLICATION_MESSAGES_RESP_STATUS_OK,
                                                                                            .position_requested = 0};

    if (p_event_data && event_data_size)
    {
        user_application_state_machine_event_data_t *event_data = (user_application_state_machine_event_data_t *)(p_event_data);

        switch (user_application_state_machine_state)
        {
            case USER_APPLICATION_STATE_MACHINE_STATE_CALIBRATION:
            {
                switch (event_data->type)
                {
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL:
                    {
                        stepper_motor_drv_drive_clockwise();
                    }
                    break;

                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_PROCESS_CMD_REQUEST:
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_STEP_ACTIVITY:
                    {
                        // Do not process any set angle processing requests as well as step activity events during the stepper motor position calibration phase/state
                    }
                    break;

                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_HOME_ACTIVITY:
                    {
                        stepper_motor_drv_drive_stop();

                        user_application_state_machine_state = USER_APPLICATION_STATE_MACHINE_STATE_READY;
                    }
                    break;

                    default:
                    {
                        // ERR_INVALID_EVENT_TYPE_IN_CALIBRATION_STATE
                    }
                    break;
                }
            }
            break;

            case USER_APPLICATION_STATE_MACHINE_STATE_READY:
            {
                switch (event_data->type)
                {
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_PROCESS_CMD_REQUEST:
                    {
                        user_application_state_machine_process_cmd_request_event_data_u *process_cmd_request_event_data = (user_application_state_machine_process_cmd_request_event_data_u *)(&event_data->event_specific_data.process_cmd_request_event_data);
                        user_application_messages_cmd_type_e request_cmd_type = user_application_messages_get_cmd_type(process_cmd_request_event_data);
                        switch (request_cmd_type)
                        {
                            case USER_APPLICATION_MESSAGES_CMD_TYPE_SET_STEPPER_MOTOR_ANGLE:
                            {
                                user_application_messages_cmd_set_stepper_motor_angle_t *cmd_set_stepper_motor_angle = (user_application_messages_cmd_set_stepper_motor_angle_t *)(process_cmd_request_event_data);

                                user_application_state_machine_params.saved_cmd_type = cmd_set_stepper_motor_angle->cmd_type;
                                user_application_state_machine_params.position_requested = ENCODER_DRV_ENCODER_ANGLE_TO_POSITION(cmd_set_stepper_motor_angle->angle_requested);

                                if (!ENCODER_DRV_IS_ENCODER_POSITION_VALID(user_application_state_machine_params.position_requested))
                                {
                                    user_application_state_machine_params.resp_status = USER_APPLICATION_MESSAGES_RESP_STATUS_INVALID_ANGLE_REQUESTED;

                                    user_application_state_machine_state = USER_APPLICATION_STATE_MACHINE_STATE_EXECUTION;

                                    user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL};
                                    scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
                                }
                                else
                                {
                                    user_application_state_machine_params.resp_status = USER_APPLICATION_MESSAGES_RESP_STATUS_OK;

                                    encoder_drv_encoder_position_t current_encoder_position;
                                    encoder_drv_get_encoder_position(&current_encoder_position);

                                    user_application_state_machine_state = USER_APPLICATION_STATE_MACHINE_STATE_EXECUTION;

                                    if (user_application_state_machine_params.position_requested > current_encoder_position)
                                    {
                                        stepper_motor_drv_drive_anticlockwise();
                                    }
                                    else if (user_application_state_machine_params.position_requested < current_encoder_position)
                                    {
                                        stepper_motor_drv_drive_clockwise();
                                    }
                                    else
                                    {
                                        user_application_state_machine_event_data_t state_machine_event_data = {.type = USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL};
                                        scheduler_event_put(&state_machine_event_data, sizeof(state_machine_event_data), user_application_state_machine_event_handler);
                                    }
                                }
                            }
                            break;

                            default:
                            {
                                // ERR_INVALID_REQUEST_CMD_TYPE
                            }
                            break;
                        }
                        
                    }
                    break;

                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_STEP_ACTIVITY:
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_HOME_ACTIVITY:
                    {
                        // Do not process any step or home activity events while in the ready state (this may occur if the stepper motor rotor has been forcibly rotated by someone/external factors)
                    }
                    break;

                    default:
                    {
                        // ERR_INVALID_EVENT_TYPE_IN_READY_STATE
                    }
                    break;
                }
            }
            break;

            case USER_APPLICATION_STATE_MACHINE_STATE_EXECUTION:
            {
                switch (event_data->type)
                {
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_PROCESS_CMD_REQUEST:
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_HOME_ACTIVITY:
                    {
                        // Ignore any set angle process requests as well as home activity events during the stepper motor set angle request execution state/driving phase
                    }
                    break;

                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_INTERNAL:
                    case USER_APPLICATION_STATE_MACHINE_EVENT_TYPE_STEP_ACTIVITY:
                    {
                        switch (user_application_state_machine_params.saved_cmd_type)
                        {
                            case USER_APPLICATION_MESSAGES_CMD_TYPE_SET_STEPPER_MOTOR_ANGLE:
                            {
                                encoder_drv_encoder_position_t current_encoder_position;
                                encoder_drv_get_encoder_position(&current_encoder_position);

                                if ((user_application_state_machine_params.position_requested == current_encoder_position) || (user_application_state_machine_params.resp_status != USER_APPLICATION_MESSAGES_RESP_STATUS_OK))
                                {
                                    stepper_motor_drv_drive_stop();

                                    user_application_messages_resp_set_stepper_motor_angle_t resp_set_stepper_motor_angle = {.resp_type = USER_APPLICATION_MESSAGES_RESP_TYPE_SET_STEPPER_MOTOR_ANGLE,
                                                                                                                             .resp_status = user_application_state_machine_params.resp_status,
                                                                                                                             .angle_achieved = ENCODER_DRV_ENCODER_POSITION_TO_ANGLE(user_application_state_machine_params.position_requested)};

                                    ipc_send_data_packet(&resp_set_stepper_motor_angle, sizeof(resp_set_stepper_motor_angle));

                                    user_application_state_machine_state = USER_APPLICATION_STATE_MACHINE_STATE_READY;
                                }
                            }
                            break;

                            default:
                            {
                                // ERR_INVALID_REQUEST_CMD_TYPE
                            }
                            break;
                        }
                    }
                    break;

                    default:
                    {
                        // ERR_INVALID_EVENT_TYPE_IN_EXECUTION_STATE
                    }
                    break;
                }
            }
            break;

            default:
            {
                // ERR_INVALID_USER_APPLICATION_STATE_MACHINE_STATE
            }
            break;
        }
    }
}