import numpy as np
from app_gui import AppGui
from model import Model


class Controller:
    """ Class responsible for handling requests from GUI """

    def __init__(self, view: AppGui, model: Model) -> None:
        """
        Set reference to GUI class and init UART connection
        :param view: reference to the GUI of the application
        :param model: reference to the model of the application
        """
        self.model: Model = model
        self.view: AppGui = view

    def connect_signals_to_slots(self):
        """ Connect signals from buttons with their slots """
        self.view.temperature_button.clicked.connect(self.handle_request_temperature)
        self.view.humidity_button.clicked.connect(self.handle_request_humidity)
        self.view.pressure_button.clicked.connect(self.handle_request_pressure)
        self.view.temperature_plot_button.clicked.connect(self.handle_temperature_plot)
        self.view.humidity_plot_button.clicked.connect(self.handle_humidity_plot)
        self.view.pressure_plot_button.clicked.connect(self.handle_pressure_plot)
        self.view.set_sampling_rate_button.clicked.connect(self.handle_setting_sampling_rate)
        self.view.get_sampling_rate_button.clicked.connect(self.handle_getting_sample_rate)

    def handle_request_temperature(self) -> None:
        """ Request last temperature measurement and put result in the 'Temperature label' box """
        result: str = self.model.get_last_temperature()
        self.view.temperature_label.setText(f"{result} deg C")

    def handle_request_humidity(self) -> None:
        """ Request last humidity measurement and put result in the 'Humidity label' box """
        result: str = self.model.get_last_humidity()
        self.view.humidity_label.setText(f"{result} %")

    def handle_request_pressure(self) -> None:
        """ Request last pressure measurement and put result in the 'Pressure label' box """
        result: str = self.model.get_last_pressure()
        self.view.pressure_label.setText(f"{result} hPa")

    def handle_temperature_plot(self) -> None:
        """ Request temperature measurements from the data buffer and plot it in the 'Temperature plot' widget """
        self.view.temperature_plot.clear()
        y_axis_val = self.model.get_temperature_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.temperature_plot.plot(x_axis_val, y_axis_val, pen='r')

    def handle_humidity_plot(self) -> None:
        """ Request humidity measurements from the data buffer and plot it in the 'Humidity plot' widget """
        self.view.humidity_plot.clear()
        y_axis_val = self.model.get_humidity_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.humidity_plot.plot(x_axis_val, y_axis_val, pen='g')

    def handle_pressure_plot(self) -> None:
        """ Request humidity measurements from the data buffer and plot it in the 'Pressure plot' widget """
        self.view.pressure_plot.clear()
        y_axis_val = self.model.get_pressure_values()
        x_axis_val = np.array([num for num in range(len(y_axis_val))])
        self.view.pressure_plot.plot(x_axis_val, y_axis_val, pen='b')

    def handle_setting_sampling_rate(self) -> None:
        """
        Get text from 'Sampling rate line edit' box, and send command setting sample rate
        """
        try:
            sample_rate: int = int(self.view.sampling_rate_line_edit.text())
        except ValueError:
            self.view.show_error_message("Incorrect sampling rate provided")
        else:
            self.model.set_sampling_rate(sample_rate)

    def handle_getting_sample_rate(self) -> None:
        """
        Request information about sampling rate and put it in the 'Current sampling rate label' label
        """
        self.view.current_sampling_rate_label.setText(self.model.get_sampling_rate())
