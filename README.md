# Closed Loop Stepper Motor Control

## Overview

This project enables precise control over the position of a stepper motor using a Raspberry Pi Pico (RP2040) microcontroller unit (MCU). This project implements a bidirectional communication system between the MCU and a PC, allowing for real-time monitoring and control of the stepper motor's movement. The system is built upon an event-driven architecture, which efficiently manages asynchronous events and tasks to ensure smooth operation. Key components include drivers for stepper motors and encoders, components like the Scheduler for managing system tasks and an Inter Processor Communication (IPC) module for seamless data exchange between the MCU and the PC.

## Hardware Setup

As mentioned before, the entire thing is implemented on the Raspberry Pi Pico (RP2040) microcontroller, which features 2 MB of FLASH and 264 KB of RAM and operates at speeds of up to 133 MHz. A crucial aspect of the setup involves communication with the host computer via USB, which not only powers the device but also establishes the communication link. External components, including the incremental encoder and the NEMA 17 stepper motor, are seamlessly interfaced through GPIO pins, alongside the DRV8825 stepper motor driver module. Last but not least, the stepper motor and the DRV8825 driver circuitry are both powered by a 12V power supply.

## Drivers

1. **Stepper Motor Driver**: Implements the stepper motor drive functionality for driving a stepper motor via the DRV8825 driver. This module generates PWM step pulses to drive the stepper motor in various step modes, including full-step, half-step and microstepping. It provides functions for initializing the PWM hardware, setting up the step mode and controlling the direction and speed of the stepper motor. During initialization, it calculates the required PWM frequency based on the desired step speed (the duty cycle has been set to 50%).

2. **Encoder Driver**: Implements the incremental encoder driver functionality into the closed-loop feedback system, enabling accurate position tracking and motion detection for stepper motors. Utilizing an interrupt-driven approach, it detects encoder transitions and thus updates motor position data in real-time. By synchronizing motor movements with encoder signals, it ensures precise step positioning and thus provides functions for configuring encoder pins, reading encoder output signals and calculating motor position based on encoder feedback. It also provides flexibility in the way events are handled, allowing immediate or scheduled execution of user defined event handlers upon step or home activity events, thus enabling efficient synchronization with other system processes based on the user preference.

## Components

1. **Inter Processor Communication**: Implements an inter processor communication functionality to enable seamless data exchange between the Raspberry Pi Pico microcontroller and external systems or user interfaces, such as a PC. Operating over a serial communication interface (USB), this module employs a packet based protocol to facilitate bidirectional communication, packet formatting and error handling during real-time data transmission. Upon initialization, the module configures settings and sets up the user event handler. Throughout transmission and reception, it ensures data integrity through header construction and CRC checksum computation. Incoming data is managed by a state machine implementation, which progresses through stages/states including a packet header parsing, data length extraction and CRC verification at the end of the data packet. Subsequently, post processing occurs upon receiving a complete and valid packet, with CRC verification and data preparation for further processing based on the user provided event handler.

2. **Scheduler**: Implements a task scheduler for managing as well as scheduling asynchronous events and tasks within the system. It has been developed as a prerequisite for implementing a fully event-driven system that ensures timely sequential processing of critical tasks and interrupts while minimizing overhead. The module provides functions for initializing the scheduler, registering/adding tasks to the queue and handling task execution. It maintains a queue of events, each consisting of an event handler and optional data, ensuring that the data size adheres to defined limits and executes them iterating through each event and invoking its associated event handler with the corresponding data.

## Application

Implements the user application for controlling and managing tasks related to stepper motor control and encoder feedback processing in conjunction with external communication with the PC via the IPC module. The application layer utilizes a state machine approach to manage its operation across different states, including initial stepper motor/encoder position calibration, ready and execution phases. Upon initialization, it configures various system components, such as the IPC and encoder drivers, setting up event handlers for data processing, as well as the stepper motor driver and the scheduler component. This layer handles events such as processing command requests received from the PC, stepper motor step activities and home activities, each triggering specific actions within the system. For instance, during the calibration phase, the module drives the stepper motor to calibrate its position based on the encoder feedback. In the ready phase, it processes command requests to set the stepper motor angle while ensuring the validity of the request and responding back with appropriate status messages. Throughout execution, it continuously monitors the stepper motor's movement, halting when the desired angle is achieved or responding to any unexpected events. This approach ensures efficient and reliable operation of the user application, facilitating seamless interaction with an external system (PC).

## PC Stepper Motor Commander

Implements the Stepper Motor Commander (SMC) designed to control stepper motor angle/movement through serial communication with an external system (MCU), employing the Inter Processor Communication (IPC) module. It requires an input argument indicating the desired angle for the stepper motor's rotation. After validating the desired angle's range, it establishes communication using the IPC module to transmit a command frame for setting the stepper motor angle. This frame includes a command type and the requested angle data. Following the transmission, the SMC awaits a response frame from the MCU side. Upon receipt, it validates the response type and status to confirm successful execution. If successful, it retrieves the achieved angle from the response and displays it. This process ensures precise control and automation of the stepper motor control over a PC interface.

### Usage

Follow these steps to use the Python script (`main.py`) to control the stepper motor angle:

1. **Navigate to the directory:** Open a terminal and navigate to the directory containing the `main.py` file.

2. **Execute the script:** Run the following command, replacing `<desired_angle>` with the angle you want to set for the stepper motor in degrees (the requested angle should be within the range of 0 to 359 degrees):

    ```
    python main.py <desired_angle>
    ```

4. **Wait for confirmation:** The script will establish communication with the MCU via serial communication using the IPC module. Wait for the response from the MCU confirming the successful execution of the command.

5. **Interpret the output:** If successful, the script will display the achieved angle of the stepper motor.

## Additional Notes

1. **System Watchdog**: The system watchdog timeout duration (currently set to 1000 ms) as well as the debug watchdog pause option can be changed in the main header.

2. **Inter Processor Communication**: The maximum IPC packet transaction size has been set to 255 bytes, thus limiting the maximum number of data bytes to 251 bytes, which can be reduced if your application doesn't require handling such large data packets. Also, the timeout for reading the receiver buffer has been set to 0. Both of these settings can be changed in the IPC header. These changes should also be reflected in the PC IPC module, which is being used by the stepper motor commander (SMC).

3. **System Clock**: The system clock can be changed by just updating the CPU core clock frequency setting in the system configuration header (right now it has been set to 133 MHz). Just make sure that the new set frequency is achievable using the vcocalc.py script provided in the Raspberry Pi Pico SDK.

4. **Scheduler**: The scheduler is dependent on the maximum queue size as well as the maximum data buffer size; both of which can be changed in the scheduler header. They should be updated based on the maximum amount of data generated for processing during a particular event in the system, as well as on how busy and CPU intensive the application is.

5. **Stepper Motor Driver**: The GPIO pin mappings for the DRV8825 stepper motor driver, as well as the PWM clock frequency divider, can be changed in the stepper motor driver header.

6. **Encoder Driver**: The GPIO pin mappings for the encoder, the encoder step resolution, as well as position bounds, can all be changed in the encoder driver header.

7. **User Application**: Within the user application, the encoder driver event handling has been configured for immediate response, ensuring real-time synchronization with motor movements. Additionally, the stepper motor's step mode has been set to 1/16 microstepping mode, enhancing precision and smoothness of motion. Moreover, the step speed has been set to 2000 steps per second, providing a balance between speed and accuracy; this setting can be adjusted as needed in the user application header.

## Dependencies

1. **Raspberry Pi Pico SDK**: Provides essential libraries, headers and tools for developing firmware code and applications for the Raspberry Pi Pico microcontroller platform.