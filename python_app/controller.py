from uart_handler import UartHandler
from app_gui import AppGui


class Controller:

    def __init__(self, view: AppGui):
        self.view = view
        self.uart_handler = UartHandler()

    def handle_request_temperature(self):
        self.uart_handler.send_command("get temp")
        result = self.uart_handler.read_last_data()
        self.view.temperature_label.setText(f"{result} deg C")
