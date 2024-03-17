#include "drivers/encoder/encoder_drv.h"

static encoder_drv_event_handlers_t encoder_drv_event_handlers = {.step_activity_event_handler = NULL,
                                                                  .home_activity_event_handler = NULL};
static encoder_drv_event_handling_e encoder_drv_event_handling = ENCODER_DRV_EVENT_HANDLING_SCHEDULED;
static encoder_drv_encoder_position_t encoder_drv_encoder_position = 0;

static void encoder_drv_step_home_pin_transitions_callback(uint32_t gpio, uint32_t event_mask);

static void encoder_drv_increment_encoder_position(void);
static void encoder_drv_decrement_encoder_position(void);

encoder_drv_operation_status_e encoder_drv_init(encoder_drv_init_params_t init_params)
{
    scheduler_init();

    if ((init_params.event_handling != ENCODER_DRV_EVENT_HANDLING_IMMEDIATE) && (init_params.event_handling != ENCODER_DRV_EVENT_HANDLING_SCHEDULED))
    {
        return ENCODER_DRV_OPERATION_STATUS_INVALID_EVENT_HANDLING;
    }

    gpio_init(ENCODER_DRV_PIN_A_ANTICLOCKWISE);
    gpio_init(ENCODER_DRV_PIN_B_CLOCKWISE);
    gpio_init(ENCODER_DRV_PIN_Z_HOME);

    gpio_set_dir(ENCODER_DRV_PIN_A_ANTICLOCKWISE, GPIO_IN);
    gpio_set_dir(ENCODER_DRV_PIN_B_CLOCKWISE, GPIO_IN);
    gpio_set_dir(ENCODER_DRV_PIN_Z_HOME, GPIO_IN);

    gpio_set_irq_enabled_with_callback(ENCODER_DRV_PIN_A_ANTICLOCKWISE, GPIO_IRQ_EDGE_RISE, true, &encoder_drv_step_home_pin_transitions_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_DRV_PIN_B_CLOCKWISE, GPIO_IRQ_EDGE_RISE, true, &encoder_drv_step_home_pin_transitions_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_DRV_PIN_Z_HOME, GPIO_IRQ_EDGE_RISE, true, &encoder_drv_step_home_pin_transitions_callback);

    encoder_drv_event_handlers = init_params.event_handlers;
    encoder_drv_event_handling = init_params.event_handling;

    return ENCODER_DRV_OPERATION_STATUS_OK;
}

encoder_drv_operation_status_e encoder_drv_get_encoder_position(encoder_drv_encoder_position_t *p_encoder_position)
{
    *p_encoder_position = encoder_drv_encoder_position;

    return ENCODER_DRV_OPERATION_STATUS_OK;
}

encoder_drv_operation_status_e encoder_drv_set_encoder_position(encoder_drv_encoder_position_t *p_encoder_position)
{
    if (!ENCODER_DRV_IS_ENCODER_POSITION_VALID(*p_encoder_position))
    {
        return ENCODER_DRV_OPERATION_STATUS_INVALID_POSITION;
    }

    system_utils_disable_interrupts();
    encoder_drv_encoder_position = *p_encoder_position;
    system_utils_enable_interrupts();

    return ENCODER_DRV_OPERATION_STATUS_OK;
}

static void encoder_drv_step_home_pin_transitions_callback(uint32_t gpio, uint32_t event_mask)
{
    static bool recently_encountered_home_event = 0;

    switch (gpio)
    {
        case ENCODER_DRV_PIN_A_ANTICLOCKWISE:
        case ENCODER_DRV_PIN_B_CLOCKWISE:
        {
            if (event_mask == GPIO_IRQ_EDGE_RISE)
            {
                static bool previous_anticlockwise_step_event_assertion_failed = 1;
                static bool previous_clockwise_step_event_assertion_failed = 1;
                static bool encoder_position_update_notification = 0;

                switch (gpio)
                {
                    case ENCODER_DRV_PIN_A_ANTICLOCKWISE:
                    {
                        if (gpio_get(ENCODER_DRV_PIN_B_CLOCKWISE))
                        {
                            if (previous_clockwise_step_event_assertion_failed)
                            {
                                if (!recently_encountered_home_event)
                                {
                                    encoder_drv_decrement_encoder_position();
                                }

                                recently_encountered_home_event = 0;
                                encoder_position_update_notification = 1;
                            }

                            previous_clockwise_step_event_assertion_failed = 1;
                        }
                        else
                        {
                            if (!recently_encountered_home_event)
                            {
                                encoder_drv_increment_encoder_position();
                            }

                            recently_encountered_home_event = 0;
                            encoder_position_update_notification = 1;
                            previous_anticlockwise_step_event_assertion_failed = 0;
                        }
                    }
                    break;

                    case ENCODER_DRV_PIN_B_CLOCKWISE:
                    {
                        if (gpio_get(ENCODER_DRV_PIN_A_ANTICLOCKWISE))
                        {
                            if (previous_anticlockwise_step_event_assertion_failed)
                            {
                                if (!recently_encountered_home_event)
                                {
                                    encoder_drv_increment_encoder_position();
                                }

                                recently_encountered_home_event = 0;
                                encoder_position_update_notification = 1;
                            }

                            previous_anticlockwise_step_event_assertion_failed = 1;
                        }
                        else
                        {
                            if (!recently_encountered_home_event)
                            {
                                encoder_drv_decrement_encoder_position();
                            }

                            recently_encountered_home_event = 0;
                            encoder_position_update_notification = 1;
                            previous_clockwise_step_event_assertion_failed = 0;
                        }
                    }
                    break;

                    default:
                    {
                        // ERR_INVALID_ENCODER_DRV_PIN
                    }
                    break;
                }

                if (encoder_drv_event_handlers.step_activity_event_handler && encoder_position_update_notification)
                {
                    encoder_position_update_notification = 0;

                    switch (encoder_drv_event_handling)
                    {
                        case ENCODER_DRV_EVENT_HANDLING_IMMEDIATE:
                        {
                            encoder_drv_event_handlers.step_activity_event_handler(NULL, 0);
                        }
                        break;

                        case ENCODER_DRV_EVENT_HANDLING_SCHEDULED:
                        {
                            scheduler_event_put(NULL, 0, encoder_drv_event_handlers.step_activity_event_handler);
                        }
                        break;

                        default:
                        {
                            // ERR_INVALID_ENCODER_DRV_EVENT_HANDLING
                        }
                        break;
                    }
                }
            }
        }
        break;

        case ENCODER_DRV_PIN_Z_HOME:
        {
            if (event_mask == GPIO_IRQ_EDGE_RISE)
            {
                recently_encountered_home_event = 1;

                system_utils_disable_interrupts();
                encoder_drv_encoder_position = 0;
                system_utils_enable_interrupts();

                if (encoder_drv_event_handlers.home_activity_event_handler)
                {
                    switch (encoder_drv_event_handling)
                    {
                        case ENCODER_DRV_EVENT_HANDLING_IMMEDIATE:
                        {
                            encoder_drv_event_handlers.home_activity_event_handler(NULL, 0);
                        }
                        break;

                        case ENCODER_DRV_EVENT_HANDLING_SCHEDULED:
                        {
                            scheduler_event_put(NULL, 0, encoder_drv_event_handlers.step_activity_event_handler);
                        }
                        break;

                        default:
                        {
                            // ERR_INVALID_ENCODER_DRV_EVENT_HANDLING
                        }
                        break;
                    }
                }
            }
        }
        break;

        default:
        {
            // ERR_UNCONFIGURED_GPIO_TRANSITION_EVENT
        }
        break;
    }
}

static void encoder_drv_increment_encoder_position(void)
{
    system_utils_disable_interrupts();

    // Avoiding the use of modulo operator since this function is being called in an ISR
    // encoder_drv_encoder_position = (++encoder_drv_encoder_position) % ENCODER_DRV_ENCODER_RESOLUTION;

    if (encoder_drv_encoder_position == ENCODER_DRV_ENCODER_POSITION_HIGHER_BOUND)
    {
        encoder_drv_encoder_position = ENCODER_DRV_ENCODER_POSITION_LOWER_BOUND;
    }
    else
    {
        encoder_drv_encoder_position++;
    }

    system_utils_enable_interrupts();
}
static void encoder_drv_decrement_encoder_position(void)
{
    system_utils_disable_interrupts();

    // Avoiding the use of modulo operator since this function is being called in an ISR
    // encoder_drv_encoder_position = (--encoder_drv_encoder_position + ENCODER_DRV_ENCODER_RESOLUTION) % ENCODER_DRV_ENCODER_RESOLUTION;

    if (encoder_drv_encoder_position == ENCODER_DRV_ENCODER_POSITION_LOWER_BOUND)
    {
        encoder_drv_encoder_position = ENCODER_DRV_ENCODER_POSITION_HIGHER_BOUND;
    }
    else
    {
        encoder_drv_encoder_position--;
    }

    system_utils_enable_interrupts();
}