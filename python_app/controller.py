from uart_handler import UartHandler
from app_gui import AppGui


class Controller:
    """ Class responsible for performing requests """

    def __init__(self, view: AppGui):
        """
        Set reference to GUI class and init UART connection
        :param view: reference to the GUI of the application
        """
        self.view = view
        self.uart_handler = UartHandler()

    def handle_request_temperature(self):
        """ Action performed when requesting last temperature measurement """
        self.uart_handler.send_command("get temp")
        result = self.uart_handler.read_last_data()
        self.view.temperature_label.setText(f"{result} deg C")

    def handle_request_humidity(self):
        """ Action performed when requesting last humidity measurement """
        self.uart_handler.send_command("get hum")
        result = self.uart_handler.read_last_data()
        self.view.humidity_label.setText(f"{result} %")

    def handle_request_pressure(self):
        """ Action performed when requesting last pressure measurement """
        self.uart_handler.send_command("get press")
        result = self.uart_handler.read_last_data()
        self.view.pressure_label.setText(f"{result} hPa")
