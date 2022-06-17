import serial


class UartHandler(serial.Serial):
    """ Class responsible for UART connection """

    def __init__(self):
        """ Init connection with UART """
        super().__init__(port='COM3', baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                         stopbits=serial.STOPBITS_ONE, timeout=None)

    def send_command(self, command: str) -> None:
        """
        Write a command to UART
        :param command - command to be sent to UART (without '\n' or '\r' chars)
        """
        self.reset_input_buffer()
        self.write(f"{command}\n".encode())

    def read_last_data(self) -> str:
        """
        Read last line put on UART
        :return: last line put on UART in the form of string
        """
        read_line: str = self.readline().decode().strip()
        self.reset_output_buffer()
        return read_line
