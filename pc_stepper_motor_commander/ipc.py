# Inter Processor Communication (IPC)

import serial
import time

IPC_PACKET_MAX_TRANSACTION_SIZE = 0xFF

IPC_POST_INITIALIZATION_WAIT_TIME = 2
IPC_PACKET_HEADER_PARAM = [0xA5, 0x5A]
IPC_PACKET_HEADER_PARAM_SIZE = 2
IPC_PACKET_DATA_LENGTH_PARAM_SIZE = 1
IPC_PACKET_DATA_CRC_PARAM_SIZE = 1
IPC_PACKET_MAX_DATA_LENGTH = (IPC_PACKET_MAX_TRANSACTION_SIZE - IPC_PACKET_HEADER_PARAM_SIZE - IPC_PACKET_DATA_LENGTH_PARAM_SIZE - IPC_PACKET_DATA_CRC_PARAM_SIZE)

class IpcHandler:
    def __init__(self, port, baud_rate):
        self.pc_transceiver = serial.Serial(port, baud_rate)
        time.sleep(IPC_POST_INITIALIZATION_WAIT_TIME)

    def calculate_crc(self, data_packet):
        xor_crc = 0
        for data in data_packet:
            xor_crc ^= data
        return xor_crc

    def send_data_packet(self, data_packet):
        formatted_data_packet = bytes(IPC_PACKET_HEADER_PARAM + [len(data_packet)] + data_packet + [self.calculate_crc(data_packet)])

        try:
            self.pc_transceiver.write(formatted_data_packet)

        except Exception as e:
            print(f"Error: Failed to send the data packet: {e}")

    def receive_data_packet(self):
        try:
            if list(self.pc_transceiver.read(IPC_PACKET_HEADER_PARAM_SIZE)) != IPC_PACKET_HEADER_PARAM:
                print("Error: Encountered an invalid header in the received data")
                return None

            expected_data_length = int.from_bytes(self.pc_transceiver.read(IPC_PACKET_DATA_LENGTH_PARAM_SIZE), byteorder = "little")

            if expected_data_length > IPC_PACKET_MAX_DATA_LENGTH:
                print("Error: Expected data length exceeds the maximum supported transaction data length")
                return None

            received_data_packet = self.pc_transceiver.read(expected_data_length)

            expected_data_crc = int.from_bytes(self.pc_transceiver.read(IPC_PACKET_DATA_CRC_PARAM_SIZE), byteorder = "little")

            if expected_data_crc != self.calculate_crc(received_data_packet):
                print("Error: Expected data CRC doesn't match the CRC of the actual received data")
                return None

            return list(received_data_packet)

        except Exception as e:
            print(f"Error: Failed to receive the data packet: {e}")
            return None
