import serial


class UartHandler(serial.Serial):
    """ Wrapper around serial.Serial class """

    def __init__(self):
        """ Init connection with UART """
        # timeout in seconds
        super().__init__(port='COM12', baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                         stopbits=serial.STOPBITS_ONE)

    def send_command(self, command: str) -> None:
        self.write(f"{command}\r\n".encode())

    def read_last_data(self) -> str:
        return self.readline().decode().strip()
