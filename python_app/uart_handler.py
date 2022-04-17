import serial


class UartHandler(serial.Serial):
    """ Class responsible for UART connection """

    def __init__(self):
        """ Init connection with UART """
        super().__init__(port='COM10', baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                         stopbits=serial.STOPBITS_ONE)

    def send_command(self, command: str) -> None:
        """
        Write a command to UART
        :param command - command to be sent to UART (without '\n' or '\r' chars)
        """
        self.write(f"{command}\r\n".encode())

    def read_last_data(self) -> str:
        """
        Read last line put on UART
        :return: last line put on UART in the form of string
        """
        return self.readline().decode().strip()
