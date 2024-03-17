# Stepper Motor Commander (SMC)

import sys
from ipc import IpcHandler

def main():
    # Communication port parameters
    SMC_COMM_PORT = "/dev/ttyACM0"
    SMC_COMM_BAUD_RATE = 115200

    # Input arguments parameters
    SMC_INPUT_ARGUMENTS_MIN_COUNT = 1
    SMC_INPUT_ARGUMENTS_REQ_ANGLE_INDEX = 1

    # Requested stepper motor angle parameters
    SMC_STEPPER_MOTOR_ANGLE_LOWER_BOUND = 0
    SMC_STEPPER_MOTOR_ANGLE_HIGHER_BOUND = 359
    SMC_STEPPER_MOTOR_ANGLE_DATA_SIZE = 2

    # Command and response message parameters 
    SMC_CMD_MSG_CMD_TYPE_DATA_INDEX = 0
    SMC_CMD_MSG_CMD_TYPE_SET_STEPPER_MOTOR_ANGLE = 0
    SMC_CMD_MSG_REQ_STEPPER_MOTOR_ANGLE_DATA_INDEX = 1

    SMC_RESP_MSG_RESP_TYPE_DATA_INDEX = 0
    SMC_RESP_MSG_RESP_TYPE_SET_STEPPER_MOTOR_ANGLE = 0
    SMC_RESP_MSG_RESP_STATUS_DATA_INDEX = 1
    SMC_RESP_MSG_RESP_STATUS_OK = 0
    SMC_RESP_MSG_RESP_STATUS_INVALID_ANGLE_REQUESTED = 1
    SMC_RESP_MSG_ACH_STEPPER_MOTOR_ANGLE_DATA_INDEX = 2

    if len(sys.argv) < (SMC_INPUT_ARGUMENTS_MIN_COUNT + 1):
        raise Exception("Please provide an angle to drive the stepper motor")

    smc_requested_angle = int(sys.argv[SMC_INPUT_ARGUMENTS_REQ_ANGLE_INDEX])

    if not (SMC_STEPPER_MOTOR_ANGLE_LOWER_BOUND <= smc_requested_angle <= SMC_STEPPER_MOTOR_ANGLE_HIGHER_BOUND):
        raise Exception(f"Requested stepper motor angle should be within a range of {SMC_STEPPER_MOTOR_ANGLE_LOWER_BOUND} to {SMC_STEPPER_MOTOR_ANGLE_HIGHER_BOUND}")

    smc_ipc_handler = IpcHandler(SMC_COMM_PORT, SMC_COMM_BAUD_RATE)
    smc_command_frame = [SMC_CMD_MSG_CMD_TYPE_SET_STEPPER_MOTOR_ANGLE] + list(smc_requested_angle.to_bytes(SMC_STEPPER_MOTOR_ANGLE_DATA_SIZE, byteorder = "little"))

    print("Executing set stepper motor angle command...\n")
    smc_ipc_handler.send_data_packet(smc_command_frame)

    smc_response_frame = smc_ipc_handler.receive_data_packet()
    if smc_response_frame:
        if (smc_response_frame[SMC_RESP_MSG_RESP_TYPE_DATA_INDEX] != SMC_RESP_MSG_RESP_TYPE_SET_STEPPER_MOTOR_ANGLE):
            raise Exception("Invalid response was received")

        if (smc_response_frame[SMC_RESP_MSG_RESP_STATUS_DATA_INDEX] != SMC_RESP_MSG_RESP_STATUS_OK):
            raise Exception("Invalid stepper motor angle was requested")

        smc_achieved_angle = int.from_bytes(smc_response_frame[SMC_RESP_MSG_ACH_STEPPER_MOTOR_ANGLE_DATA_INDEX : SMC_RESP_MSG_ACH_STEPPER_MOTOR_ANGLE_DATA_INDEX + SMC_STEPPER_MOTOR_ANGLE_DATA_SIZE], byteorder = "little")

        print(f"The stepper motor position has been set to {smc_achieved_angle} deg")

    else:
        print("Error: Failed to receive a proper response (possibly due to packet corruption)")

if __name__ == "__main__":
    main()