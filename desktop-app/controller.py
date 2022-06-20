import datetime
from app_gui import AppGui
from model import Model
from received_measurements import PlotInformation, ContinuousModeInformation


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
        self.continuous_measurement_enabled: bool = False

    def connect_signals_to_slots(self):
        """
        Connect signals (emitted from buttons responsible for plotting measurements) to slots (functions responsible
        for getting data from the embedded system to be plotted)
        """
        self.view.plot_button.clicked.connect(self.handle_plot)
        self.view.start_measurements_button.clicked.connect(self.handle_start_measurements)
        self.view.list_files_button.clicked.connect(self.handle_listing_files)

    def handle_plot(self) -> None:
        """ Request temperature measurements from the data buffer and plot it in the 'Temperature plot' widget """
        self._disable_continuous_measurements()
        self.view.temperature_plot.clear()
        self.view.pressure_plot.clear()
        self.view.humidity_plot.clear()
        self.model.stop_measurements()
        file_name: str = self.view.file_name_line_edit.text()
        self.view.datetime_info_label.setText(f"Start measurements at: {file_name.replace('_', ':')}")
        plot_information: PlotInformation = self.model.get_data_to_plot(file_name)
        self.view.temperature_plot.plot(plot_information.x_axis, plot_information.temperature, pen="r", symbol="o")
        self.view.pressure_plot.plot(plot_information.x_axis, plot_information.pressure, pen="g", symbol="o")
        self.view.humidity_plot.plot(plot_information.x_axis, plot_information.humidity, pen="b", symbol="o")

    def handle_start_measurements(self) -> None:
        """ Handle start measurements """
        self._disable_continuous_measurements()
        sampling_rate_as_str: str = self.view.sampling_rate_line_edit.text()  # with leading zeros
        self.model.start_measurements_to_file(sampling_rate_as_str)
        self.view.sampling_rate_info_label.setText(f"Sampling rate of BME280 set to {sampling_rate_as_str} seconds")

        self.view.timer_last_measurements.setInterval(1000)  # 1 second
        self.view.timer_last_measurements.timeout.connect(self.update_timer_callback)
        self.view.timer_last_measurements.start()

    def handle_listing_files(self) -> None:
        self._disable_continuous_measurements()
        listed_files_info: str = self.model.list_files()
        listed_files_info = listed_files_info.replace(")", ")\n")
        self.view.list_files_text_browser.setText(listed_files_info)

    def update_timer_callback(self):
        if self.view.tab_widget.currentWidget() is self.view.last_measurements_widget:
            self._enable_continuous_measurements()
            continuous_mode_info: ContinuousModeInformation = self.model.get_data_from_continuous_mode()
            self.view.last_update_label.setText(f"{datetime.datetime.now().strftime('%d-%m-%y %H:%M:%S')}")
            self.view.temperature_label.setText(f"{continuous_mode_info.temperature:.2f} deg C")
            self.view.pressure_label.setText(f"{continuous_mode_info.pressure:.2f} hPa")
            self.view.humidity_label.setText(f"{continuous_mode_info.humidity:.2f} %")
        else:
            self._disable_continuous_measurements()

    def _enable_continuous_measurements(self):
        if not self.continuous_measurement_enabled:
            self.model.toggle_continuous_measurements()
            self.continuous_measurement_enabled = True

    def _disable_continuous_measurements(self):
        if self.continuous_measurement_enabled:
            self.model.toggle_continuous_measurements()
            self.continuous_measurement_enabled = False
