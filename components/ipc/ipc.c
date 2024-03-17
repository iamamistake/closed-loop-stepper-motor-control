#include "components/ipc/ipc.h"

static volatile uint8_t ipc_received_data_packet_buffer[IPC_PACKET_MAX_DATA_LENGTH];

static ipc_data_packet_received_event_handler_t ipc_data_packet_received_event_handler = NULL;

static void ipc_receiver_buffer_data_available_callback(void *param);
static void ipc_state_machine_event_handler(void *p_event_data, uint8_t event_data_size);
static void ipc_received_data_packet_post_processing_handler(void *p_event_data, uint8_t event_data_size);

ipc_operation_status_e ipc_init(ipc_init_params_t init_params)
{
    stdio_init_all();
    scheduler_init();

    stdio_set_chars_available_callback(&ipc_receiver_buffer_data_available_callback, NULL);
    ipc_data_packet_received_event_handler = init_params.data_packet_received_event_handler;

    return IPC_OPERATION_STATUS_OK;
}

ipc_operation_status_e ipc_send_data_packet(uint8_t *p_data_packet, uint8_t data_packet_length)
{
    if (!p_data_packet)
    {
        return IPC_OPERATION_STATUS_INVALID_DATA;
    }

    if (!((0 < data_packet_length) && (data_packet_length <= IPC_PACKET_MAX_DATA_LENGTH)))
    {
        return IPC_OPERATION_STATUS_INVALID_LENGTH;
    }

    // Send out the packet header parameters
    ipc_packet_header_param_frame_u packet_header_param_frame = {.header_param_raw = IPC_PACKET_HEADER_PARAM}; 
    putchar_raw(packet_header_param_frame.header_param.lsb);
    putchar_raw(packet_header_param_frame.header_param.msb);

    // Send out the data packet length parameter
    putchar_raw(data_packet_length);

    uint8_t data_packet_crc = 0;

    // Send out the data packet
    for (uint8_t data_packet_offset = 0; data_packet_offset < data_packet_length; data_packet_offset++)
    {
        putchar_raw(*(p_data_packet + data_packet_offset));
        data_packet_crc ^= *(p_data_packet + data_packet_offset);
        watchdog_update();
    }

    // Send out the data packet CRC parameter
    putchar_raw(data_packet_crc);

    return IPC_OPERATION_STATUS_OK;
}

static void ipc_receiver_buffer_data_available_callback(void *param)
{
    ipc_state_machine_event_data_t event_data = {.type = IPC_STATE_MACHINE_EVENT_TYPE_RECEIVED_DATA};

    while (1)
    {
        int16_t read_receiver_buffer_data = (int16_t) getchar_timeout_us(IPC_READ_RECEIVER_BUFFER_TIMEOUT_US);

        if (read_receiver_buffer_data >= 0)
        {
            event_data.received_data = read_receiver_buffer_data;
            ipc_state_machine_event_handler(&event_data, sizeof(event_data));
            watchdog_update();
        }
        else
        {
            break;
        }
    }
}

static void ipc_state_machine_event_handler(void *p_event_data, uint8_t event_data_size)
{
    static ipc_state_machine_state_e ipc_state_machine_state = IPC_STATE_MACHINE_STATE_IDLE;
    static ipc_state_machine_params_t ipc_state_machine_params = {.current_header_param_index = 0,
                                                                  .current_data_offset = 0,
                                                                  .expected_data_length = 0};

    if (p_event_data && event_data_size)
    {
        ipc_state_machine_event_data_t *event_data = (ipc_state_machine_event_data_t *)(p_event_data);

        switch (event_data->type)
        {
            case IPC_STATE_MACHINE_EVENT_TYPE_RECEIVED_DATA:
            {
                switch (ipc_state_machine_state)
                {
                    case IPC_STATE_MACHINE_STATE_IDLE:
                    {
                        ipc_packet_header_param_frame_u packet_header_param_frame = {.header_param_raw = IPC_PACKET_HEADER_PARAM};

                        if (!ipc_state_machine_params.current_header_param_index)
                        {
                            if (event_data->received_data == packet_header_param_frame.header_param.lsb)
                            {
                                ipc_state_machine_params.current_header_param_index++;
                            }
                        }
                        else
                        {
                            if (event_data->received_data == packet_header_param_frame.header_param.msb)
                            {
                                ipc_state_machine_state = IPC_STATE_MACHINE_STATE_GET_DATA_LENGTH;
                            }

                            ipc_state_machine_params.current_header_param_index = 0;
                        }
                    }
                    break;

                    case IPC_STATE_MACHINE_STATE_GET_DATA_LENGTH:
                    {
                        if ((0 < event_data->received_data) && (event_data->received_data <= IPC_PACKET_MAX_DATA_LENGTH))
                        {
                            ipc_state_machine_params.expected_data_length = event_data->received_data;
                            ipc_state_machine_state = IPC_STATE_MACHINE_STATE_GET_DATA;
                        }
                        else
                        {
                            ipc_state_machine_params.expected_data_length = 0;
                            ipc_state_machine_state = IPC_STATE_MACHINE_STATE_IDLE;
                        }
                    }
                    break;

                    case IPC_STATE_MACHINE_STATE_GET_DATA:
                    {
                        ipc_received_data_packet_buffer[ipc_state_machine_params.current_data_offset++] = event_data->received_data;

                        if (ipc_state_machine_params.current_data_offset == ipc_state_machine_params.expected_data_length)
                        {
                            ipc_state_machine_params.current_data_offset = 0;
                            ipc_state_machine_state = IPC_STATE_MACHINE_STATE_GET_DATA_CRC;
                        }
                    }
                    break;

                    case IPC_STATE_MACHINE_STATE_GET_DATA_CRC:
                    {
                        ipc_received_data_packet_post_processing_params_t received_data_packet_post_processing_params = {.received_data_size = ipc_state_machine_params.expected_data_length,
                                                                                                                         .expected_data_crc = event_data->received_data};
                        scheduler_event_put(&received_data_packet_post_processing_params, sizeof(received_data_packet_post_processing_params), ipc_received_data_packet_post_processing_handler);

                        ipc_state_machine_params.expected_data_length = 0;
                        ipc_state_machine_state = IPC_STATE_MACHINE_STATE_IDLE;
                    }
                    break;

                    default:
                    {
                        // ERR_INVALID_IPC_STATE_MACHINE_STATE
                    }
                    break;
                }
            }
            break;

            default:
            {
                // ERR_INVALID_IPC_STATE_MACHINE_EVENT_TYPE
            }
            break;
        }
    }
}

static void ipc_received_data_packet_post_processing_handler(void *p_event_data, uint8_t event_data_size)
{
    if (p_event_data && event_data_size)
    {
        ipc_received_data_packet_post_processing_params_t *received_data_packet_post_processing_params = (ipc_received_data_packet_post_processing_params_t *)(p_event_data);
        uint8_t received_data_packet_crc = 0;

        for (uint8_t received_data_packet_offset = 0; received_data_packet_offset < received_data_packet_post_processing_params->received_data_size; received_data_packet_offset++)
        {
            received_data_packet_crc ^= *(ipc_received_data_packet_buffer + received_data_packet_offset);
            watchdog_update();
        }

        ipc_data_packet_received_event_data_t data_packet_received_event_data = {.p_received_data = (uint8_t *) ipc_received_data_packet_buffer,
                                                                                 .received_data_size = received_data_packet_post_processing_params->received_data_size};

        if (received_data_packet_crc == received_data_packet_post_processing_params->expected_data_crc)
        {
            data_packet_received_event_data.reception_status = IPC_RECEPTION_STATUS_OK;
        }
        else
        {
            data_packet_received_event_data.reception_status = IPC_RECEPTION_STATUS_PACKET_CORRUPTED;
        }

        if (ipc_data_packet_received_event_handler)
        {
            ipc_data_packet_received_event_handler(&data_packet_received_event_data, sizeof(data_packet_received_event_data));
        }
    }
}