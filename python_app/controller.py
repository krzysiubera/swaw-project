import numpy as np
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
        self.view.temperature_plot_button.clicked.connect(self.handle_temperature_plot)
        self.view.humidity_plot_button.clicked.connect(self.handle_humidity_plot)
        self.view.pressure_plot_button.clicked.connect(self.handle_pressure_plot)

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

    def handle_temperature_plot(self):
        """ Action performed when requesting temperature plot """
        self.view.temperature_plot.clear()
        y_axis_val = self.model.get_temperature_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.temperature_plot.plot(x_axis_val, y_axis_val, pen='r')

    def handle_humidity_plot(self):
        """ Action performed when requesting temperature plot """
        self.view.humidity_plot.clear()
        y_axis_val = self.model.get_humidity_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.humidity_plot.plot(x_axis_val, y_axis_val, pen='g')

    def handle_pressure_plot(self):
        """ Action performed when requesting pressure plot """
        self.view.pressure_plot.clear()
        y_axis_val = self.model.get_pressure_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.pressure_plot.plot(x_axis_val, y_axis_val, pen='b')
