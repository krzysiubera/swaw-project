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

    def fill_default_values(self):
        self.view.temperature_label.setText(f"{self.model.get_last_temperature()} deg C")
        self.view.humidity_label.setText(f"{self.model.get_last_humidity()} %")
        self.view.pressure_label.setText(f"{self.model.get_last_pressure()} hPa")
        self.view.current_sampling_rate_label.setText(f"Measurements performed every {self.model.get_sampling_rate()}")

    def connect_signals_to_slots(self):
        """
        Connect signals (emitted from buttons responsible for plotting measurements) to slots (functions responsible
        for getting data from the embedded system to be plotted)
        """
        self.view.temperature_plot_button.clicked.connect(self.handle_temperature_plot)
        self.view.humidity_plot_button.clicked.connect(self.handle_humidity_plot)
        self.view.pressure_plot_button.clicked.connect(self.handle_pressure_plot)
        self.view.set_sampling_rate_button.clicked.connect(self.handle_setting_sampling_rate)

    def init_timer(self):
        """
        Get sampling rate from the embedded system and set it as an update rate of QTimer, connect to callback and
        start timer
        """
        current_sampling_rate_seconds = int(self.model.get_sampling_rate().split()[0])
        # this function accepts update time in milli seconds
        self.view.timer_last_measurements.setInterval(current_sampling_rate_seconds * 1000)
        self.view.timer_last_measurements.timeout.connect(self.update_timer_callback)
        self.view.timer_last_measurements.start()

    def update_timer_callback(self):
        """ Update widget with last measurements of temperature, pressure and humidity """
        if self.view.tab_widget.currentWidget() is self.view.last_measurements_widget:
            temperature = self.model.get_last_temperature()
            humidity = self.model.get_last_humidity()
            pressure = self.model.get_last_pressure()
            self.view.temperature_label.setText(f"{temperature} deg C")
            self.view.humidity_label.setText(f"{humidity} %")
            self.view.pressure_label.setText(f"{pressure} hPa")

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
        Get text from 'Sampling rate line edit' box, reconfigure timer responsible for updating data on "Last
        measurements" tab, send command setting sample rate and update 'Current sampling rate' label
        """
        try:
            sample_rate: int = int(self.view.sampling_rate_line_edit.text())
        except ValueError:
            self.view.show_error_message("Incorrect sampling rate provided")
        else:
            self.view.timer_last_measurements.setInterval(sample_rate * 1000)
            self.model.set_sampling_rate(sample_rate)
            self.view.current_sampling_rate_label.setText(f"Measurements performed every "
                                                          f"{self.model.get_sampling_rate()}")
