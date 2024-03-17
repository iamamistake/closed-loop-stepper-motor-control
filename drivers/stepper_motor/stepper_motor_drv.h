#ifndef STEPPER_MOTOR_DRV_H
#define STEPPER_MOTOR_DRV_H

/* STANDARD INCLUDES */
#include <stdbool.h>
#include <stdint.h>

/* LOCAL INCLUDES */
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "common/system_config.h"

/* MACRO DEFINITIONS */
#define STEPPER_MOTOR_DRV_PIN_ENABLE                        (12)
#define STEPPER_MOTOR_DRV_PIN_M0                            (11)
#define STEPPER_MOTOR_DRV_PIN_M1                            (10)
#define STEPPER_MOTOR_DRV_PIN_M2                            (9)
#define STEPPER_MOTOR_DRV_PIN_RESET                         (7)
#define STEPPER_MOTOR_DRV_PIN_SLEEP                         (6)
#define STEPPER_MOTOR_DRV_PIN_STEP                          (5)
#define STEPPER_MOTOR_DRV_PIN_DIR                           (4)
#define STEPPER_MOTOR_DRV_PIN_FAULT                         (3)

#define STEPPER_MOTOR_DRV_PWM_CLOCK_FREQ_DIVIDER            (255)
#define STEPPER_MOTOR_DRV_MIN_STEP_SPEED                    ((((SYSTEM_CONFIG_CPU_CORE_CLOCK_FREQ_KHZ * 1000)/STEPPER_MOTOR_DRV_PWM_CLOCK_FREQ_DIVIDER)/0xFFFF) + 1)

/* ENUMERATIONS */
typedef enum
{
    STEPPER_MOTOR_DRV_OPERATION_STATUS_OK = 0,
    STEPPER_MOTOR_DRV_OPERATION_STATUS_INVALID_STEP_MODE,
    STEPPER_MOTOR_DRV_OPERATION_STATUS_UNSUPPORTED_STEP_SPEED
} stepper_motor_drv_operation_status_e;

typedef enum
{
    STEPPER_MOTOR_DRV_STEP_MODE_FULL = 0,
    STEPPER_MOTOR_DRV_STEP_MODE_HALF,
    STEPPER_MOTOR_DRV_STEP_MODE_1_4,
    STEPPER_MOTOR_DRV_STEP_MODE_1_8,
    STEPPER_MOTOR_DRV_STEP_MODE_1_16,
    STEPPER_MOTOR_DRV_STEP_MODE_1_32
} stepper_motor_drv_step_mode_e;

typedef enum
{
    STEPPER_MOTOR_DRV_MODE_PIN_MASK_M0 = (1U << 0),
    STEPPER_MOTOR_DRV_MODE_PIN_MASK_M1 = (1U << 1),
    STEPPER_MOTOR_DRV_MODE_PIN_MASK_M2 = (1U << 2)
} stepper_motor_drv_mode_pin_mask_e;

typedef enum
{
    STEPPER_MOTOR_DRV_ENABLE_PIN_STATUS_ACTIVE = 0,
    STEPPER_MOTOR_DRV_ENABLE_PIN_STATUS_INACTIVE
} stepper_motor_drv_enable_pin_status_e;

typedef enum
{
    STEPPER_MOTOR_DRV_RESET_PIN_STATUS_ACTIVE = 0,
    STEPPER_MOTOR_DRV_RESET_PIN_STATUS_INACTIVE
} stepper_motor_drv_reset_pin_status_e;

typedef enum
{
    STEPPER_MOTOR_DRV_SLEEP_PIN_STATUS_ACTIVE = 0,
    STEPPER_MOTOR_DRV_SLEEP_PIN_STATUS_INACTIVE
} stepper_motor_drv_sleep_pin_status_e;

typedef enum
{
    STEPPER_MOTOR_DRV_DIR_PIN_STATUS_ANTICLOCKWISE = 0,
    STEPPER_MOTOR_DRV_DIR_PIN_STATUS_CLOCKWISE
} stepper_motor_drv_dir_pin_status_e;

/* STRUCTURES */
typedef struct __attribute__((packed))
{
    stepper_motor_drv_step_mode_e step_mode;
    uint32_t step_speed;
} stepper_motor_drv_init_params_t;

/* INLINE FUNCTIONS */
static inline bool _STEPPER_MOTOR_DRV_PIN_STATUS(stepper_motor_drv_step_mode_e mode, stepper_motor_drv_mode_pin_mask_e mask)
{
    return ((mode & mask) != 0);
}

static inline bool STEPPER_MOTOR_DRV_M0_PIN_STATUS(stepper_motor_drv_step_mode_e mode)
{
    return _STEPPER_MOTOR_DRV_PIN_STATUS(mode, STEPPER_MOTOR_DRV_MODE_PIN_MASK_M0);
}

static inline bool STEPPER_MOTOR_DRV_M1_PIN_STATUS(stepper_motor_drv_step_mode_e mode)
{
    return _STEPPER_MOTOR_DRV_PIN_STATUS(mode, STEPPER_MOTOR_DRV_MODE_PIN_MASK_M1);
}

static inline bool STEPPER_MOTOR_DRV_M2_PIN_STATUS(stepper_motor_drv_step_mode_e mode)
{
    return _STEPPER_MOTOR_DRV_PIN_STATUS(mode, STEPPER_MOTOR_DRV_MODE_PIN_MASK_M2);
}

/* FUNCTION PROTOTYPES */
stepper_motor_drv_operation_status_e stepper_motor_drv_init(stepper_motor_drv_init_params_t init_params);
stepper_motor_drv_operation_status_e stepper_motor_drv_drive_anticlockwise(void);
stepper_motor_drv_operation_status_e stepper_motor_drv_drive_clockwise(void);
stepper_motor_drv_operation_status_e stepper_motor_drv_drive_stop(void);

#endif /* STEPPER_MOTOR_DRV_H */
