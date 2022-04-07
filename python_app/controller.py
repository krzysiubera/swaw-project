from app_gui import AppGui
from model import Model


class Controller:
    """ Class responsible for handling requests from GUI """

    def __init__(self, view: AppGui, model: Model):
        """
        Set reference to GUI class and init UART connection
        :param view: reference to the GUI of the application
        :param model: reference to the model of the application
        """
        self.model: Model = model
        self.view: AppGui = view
        self.connect_signals()

    def connect_signals(self):
        """ Connect signals from buttons with their slots """
        self.view.temperature_button.clicked.connect(self.handle_request_temperature)
        self.view.humidity_button.clicked.connect(self.handle_request_humidity)
        self.view.pressure_button.clicked.connect(self.handle_request_pressure)

    def handle_request_temperature(self):
        """ Action performed when requesting last temperature measurement """
        result: str = self.model.get_last_temperature()
        self.view.temperature_label.setText(f"{result} deg C")

    def handle_request_humidity(self):
        """ Action performed when requesting last humidity measurement """
        result: str = self.model.get_last_humidity()
        self.view.humidity_label.setText(f"{result} %")

    def handle_request_pressure(self):
        """ Action performed when requesting last pressure measurement """
        result: str = self.model.get_last_pressure()
        self.view.pressure_label.setText(f"{result} hPa")
