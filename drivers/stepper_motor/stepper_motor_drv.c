#include "drivers/stepper_motor/stepper_motor_drv.h"

stepper_motor_drv_operation_status_e stepper_motor_drv_init(stepper_motor_drv_init_params_t init_params)
{
    switch (init_params.step_mode)
    {
        case STEPPER_MOTOR_DRV_STEP_MODE_FULL:
        case STEPPER_MOTOR_DRV_STEP_MODE_HALF:
        case STEPPER_MOTOR_DRV_STEP_MODE_1_4:
        case STEPPER_MOTOR_DRV_STEP_MODE_1_8:
        case STEPPER_MOTOR_DRV_STEP_MODE_1_16:
        {
            if (init_params.step_speed >= STEPPER_MOTOR_DRV_MIN_STEP_SPEED)
            {
                gpio_init(STEPPER_MOTOR_DRV_PIN_ENABLE);
                gpio_init(STEPPER_MOTOR_DRV_PIN_M0);
                gpio_init(STEPPER_MOTOR_DRV_PIN_M1);
                gpio_init(STEPPER_MOTOR_DRV_PIN_M2);
                gpio_init(STEPPER_MOTOR_DRV_PIN_RESET);
                gpio_init(STEPPER_MOTOR_DRV_PIN_SLEEP);
                gpio_init(STEPPER_MOTOR_DRV_PIN_DIR);

                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_ENABLE, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_M0, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_M1, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_M2, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_RESET, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_SLEEP, GPIO_OUT);
                gpio_set_dir(STEPPER_MOTOR_DRV_PIN_DIR, GPIO_OUT);

                gpio_put(STEPPER_MOTOR_DRV_PIN_ENABLE, STEPPER_MOTOR_DRV_ENABLE_PIN_STATUS_ACTIVE);
                gpio_put(STEPPER_MOTOR_DRV_PIN_M0, STEPPER_MOTOR_DRV_M0_PIN_STATUS(init_params.step_mode));
                gpio_put(STEPPER_MOTOR_DRV_PIN_M1, STEPPER_MOTOR_DRV_M1_PIN_STATUS(init_params.step_mode));
                gpio_put(STEPPER_MOTOR_DRV_PIN_M2, STEPPER_MOTOR_DRV_M2_PIN_STATUS(init_params.step_mode));
                gpio_put(STEPPER_MOTOR_DRV_PIN_RESET, STEPPER_MOTOR_DRV_RESET_PIN_STATUS_INACTIVE);
                gpio_put(STEPPER_MOTOR_DRV_PIN_SLEEP, STEPPER_MOTOR_DRV_SLEEP_PIN_STATUS_INACTIVE);
                gpio_put(STEPPER_MOTOR_DRV_PIN_DIR, STEPPER_MOTOR_DRV_DIR_PIN_STATUS_ANTICLOCKWISE);

                gpio_init(STEPPER_MOTOR_DRV_PIN_STEP);

                gpio_set_function(STEPPER_MOTOR_DRV_PIN_STEP, GPIO_FUNC_PWM);

                pwm_config config = pwm_get_default_config();
                pwm_config_set_clkdiv_int(&config, STEPPER_MOTOR_DRV_PWM_CLOCK_FREQ_DIVIDER);

                pwm_init(pwm_gpio_to_slice_num(STEPPER_MOTOR_DRV_PIN_STEP), &config, false);

                uint16_t required_wrap = (((SYSTEM_CONFIG_CPU_CORE_CLOCK_FREQ_KHZ * 1000)/STEPPER_MOTOR_DRV_PWM_CLOCK_FREQ_DIVIDER)/init_params.step_speed);
                pwm_set_wrap(pwm_gpio_to_slice_num(STEPPER_MOTOR_DRV_PIN_STEP), required_wrap);

                // Configuring the STEPPER_MOTOR_DRV_PIN_STEP for 50% duty cycle with (required_wrap/2) as the high level cycles
                pwm_set_gpio_level(STEPPER_MOTOR_DRV_PIN_STEP, required_wrap/2);
            }
            else
            {
                return STEPPER_MOTOR_DRV_OPERATION_STATUS_UNSUPPORTED_STEP_SPEED;
            }
        }

        break;

        default:
        {
            return STEPPER_MOTOR_DRV_OPERATION_STATUS_INVALID_STEP_MODE;
        }
        break;
    }
}

stepper_motor_drv_operation_status_e stepper_motor_drv_drive_anticlockwise(void)
{
    gpio_put(STEPPER_MOTOR_DRV_PIN_DIR, STEPPER_MOTOR_DRV_DIR_PIN_STATUS_ANTICLOCKWISE);
    pwm_set_enabled(pwm_gpio_to_slice_num(STEPPER_MOTOR_DRV_PIN_STEP), true);

    return STEPPER_MOTOR_DRV_OPERATION_STATUS_OK;
}

stepper_motor_drv_operation_status_e stepper_motor_drv_drive_clockwise(void)
{
    gpio_put(STEPPER_MOTOR_DRV_PIN_DIR, STEPPER_MOTOR_DRV_DIR_PIN_STATUS_CLOCKWISE);
    pwm_set_enabled(pwm_gpio_to_slice_num(STEPPER_MOTOR_DRV_PIN_STEP), true);

    return STEPPER_MOTOR_DRV_OPERATION_STATUS_OK;
}

stepper_motor_drv_operation_status_e stepper_motor_drv_drive_stop(void)
{
    pwm_set_enabled(pwm_gpio_to_slice_num(STEPPER_MOTOR_DRV_PIN_STEP), false);

    return STEPPER_MOTOR_DRV_OPERATION_STATUS_OK;
}
