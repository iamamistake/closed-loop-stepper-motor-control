cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(closed-loop-stepper-motor-control C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(closed-loop-stepper-motor-control
    main.c
    common/system_config.c
    common/system_utils.c
    drivers/stepper_motor/stepper_motor_drv.c
    drivers/encoder/encoder_drv.c
    components/ipc/ipc.c
    components/scheduler/scheduler.c
    application/user_application.c
)

target_include_directories(closed-loop-stepper-motor-control
    PRIVATE
    ./
)

pico_enable_stdio_usb(closed-loop-stepper-motor-control 1)
pico_enable_stdio_uart(closed-loop-stepper-motor-control 0)

target_link_libraries(closed-loop-stepper-motor-control pico_stdlib hardware_pwm)
target_link_libraries(closed-loop-stepper-motor-control pico_stdlib)

pico_add_extra_outputs(closed-loop-stepper-motor-control)

