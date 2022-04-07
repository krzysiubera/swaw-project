from uart_handler import UartHandler


class Model:
    """ Class representing model of the application """

    def __init__(self):
        """ Init object handling UART connection """
        self.uart_handler = UartHandler()

    def get_last_temperature(self) -> str:
        """ Get last temperature from UART """
        self.uart_handler.send_command("get temp")
        return self.uart_handler.read_last_data()

    def get_last_humidity(self) -> str:
        """ Get last humidity from UART """
        self.uart_handler.send_command("get hum")
        return self.uart_handler.read_last_data()

    def get_last_pressure(self):
        """ Get last pressure from UART """
        self.uart_handler.send_command("get press")
        return self.uart_handler.read_last_data()
