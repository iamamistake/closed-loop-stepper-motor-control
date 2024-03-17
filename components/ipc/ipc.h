#ifndef IPC_H
#define IPC_H

/* STANDARD INCLUDES */
#include <stdint.h>

/* LOCAL INCLUDES */
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "components/scheduler/scheduler.h"

/* MACRO DEFINITIONS */
#define IPC_PACKET_MAX_TRANSACTION_SIZE                     (0xFF)

#define IPC_PACKET_HEADER_PARAM                             (0x5AA5)
#define IPC_PACKET_HEADER_PARAM_SIZE                        (sizeof(uint16_t))
#define IPC_PACKET_DATA_LENGTH_PARAM_SIZE                   (sizeof(uint8_t))
#define IPC_PACKET_DATA_CRC_PARAM_SIZE                      (sizeof(uint8_t))

#define IPC_PACKET_MAX_DATA_LENGTH                          (IPC_PACKET_MAX_TRANSACTION_SIZE - IPC_PACKET_HEADER_PARAM_SIZE - IPC_PACKET_DATA_LENGTH_PARAM_SIZE - IPC_PACKET_DATA_CRC_PARAM_SIZE)

#define IPC_READ_RECEIVER_BUFFER_TIMEOUT_US                 (0)

/* CALLBACK HANDLER TYPEDEFS  */
typedef void (*ipc_data_packet_received_event_handler_t)(void *p_event_data, uint8_t event_data_size);

/* ENUMERATIONS */
typedef enum
{
    IPC_OPERATION_STATUS_OK = 0,
    IPC_OPERATION_STATUS_INVALID_LENGTH,
    IPC_OPERATION_STATUS_INVALID_DATA
} ipc_operation_status_e;

typedef enum
{
    IPC_RECEPTION_STATUS_OK = 0,
    IPC_RECEPTION_STATUS_PACKET_CORRUPTED
} ipc_reception_status_e;

typedef enum
{
    IPC_STATE_MACHINE_STATE_IDLE = 0,
    IPC_STATE_MACHINE_STATE_GET_DATA_LENGTH,
    IPC_STATE_MACHINE_STATE_GET_DATA,
    IPC_STATE_MACHINE_STATE_GET_DATA_CRC
} ipc_state_machine_state_e;

typedef enum
{
    IPC_STATE_MACHINE_EVENT_TYPE_RECEIVED_DATA = 0,
} ipc_state_machine_event_type_e;

/* STRUCTURES */
typedef struct __attribute__((packed))
{
    uint8_t current_header_param_index;
    uint8_t current_data_offset;
    uint8_t expected_data_length;
} ipc_state_machine_params_t;

typedef struct __attribute__((packed))
{
    ipc_state_machine_event_type_e type;
    uint8_t received_data;
} ipc_state_machine_event_data_t;

typedef struct __attribute__((packed))
{
    ipc_data_packet_received_event_handler_t data_packet_received_event_handler;
} ipc_init_params_t;

typedef struct __attribute__((packed))
{
    uint8_t received_data_size;
    uint8_t expected_data_crc;
} ipc_received_data_packet_post_processing_params_t;

typedef struct __attribute__((packed))
{
    uint8_t reception_status;
    uint8_t *p_received_data;
    uint8_t received_data_size;
} ipc_data_packet_received_event_data_t;

/* UNIONS */
typedef union
{
    struct
    {
        uint8_t lsb;
        uint8_t msb;
    } header_param;
    uint16_t header_param_raw;
} ipc_packet_header_param_frame_u;

/* FUNCTION PROTOTYPES */
ipc_operation_status_e ipc_init(ipc_init_params_t init_params);
ipc_operation_status_e ipc_send_data_packet(uint8_t *p_data_packet, uint8_t data_packet_length);

#endif /* IPC_H */