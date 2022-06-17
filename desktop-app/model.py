import datetime
import numpy as np
from uart_handler import UartHandler
from received_measurements import PlotInformation, ContinuousModeInformation


class Model:
    """ Class representing model of the application """
    end_buffer_msg = "EOF"

    def __init__(self):
        """ Init object handling UART connection """
        self.uart_handler = UartHandler()

    def start_measurements_to_file(self, sampling_rate: str):
        start_measurements_datetime: str = datetime.datetime.now().strftime("%d-%m-%y %H_%M_%S")
        self.uart_handler.send_command(f"00;{sampling_rate};/SD:/{start_measurements_datetime}")

    def list_files(self):
        self.uart_handler.send_command("04")
        return self.uart_handler.read_last_data()

    def get_data_to_plot(self, file_name: str) -> PlotInformation:
        self.uart_handler.send_command(f"02;/SD:/{file_name}")
        received_messages: list[str] = self._receive_messages_from_sd_card()
        return self._process_messages_from_sd_card(received_messages)

    def _receive_messages_from_sd_card(self) -> list[str]:
        """
        Receive measurements from UART data buffer until 'end_buffer_msg' is received or number of received measurements
        exceeds 'max_num_measurements'
        """
        received_msgs: list[str] = []
        while True:
            received_msg: str = self.uart_handler.read_last_data()
            if received_msg == self.__class__.end_buffer_msg:
                break
            received_msgs.append(received_msg)
        return received_msgs

    @staticmethod
    def _process_messages_from_sd_card(received_messages: list[str]) -> PlotInformation:
        num_received_measurements: int = len(received_messages) - 1  # -1 accounts for "interval" message
        interval_msg: str = received_messages[0]
        interval: int = int(interval_msg.split(":")[1])
        x_axis: np.ndarray = np.array([0])
        for _ in range(num_received_measurements - 1):  # -1 accounts for adding 0 before loop
            x_axis = np.append(x_axis, x_axis[-1] + interval)

        temperature: np.ndarray = np.array([])
        pressure: np.ndarray = np.array([])
        humidity: np.ndarray = np.array([])

        for message in received_messages[1:]:
            temperature_msg, pressure_msg, humidity_msg, empty = message.split(";")
            temperature = np.append(temperature, float(temperature_msg.split(":")[1]))
            pressure = np.append(pressure, float(pressure_msg.split(":")[1]))
            humidity = np.append(humidity, float(humidity_msg.split(":")[1]))

        return PlotInformation(x_axis, temperature, pressure, humidity)

    def stop_measurements(self):
        self.uart_handler.send_command("01")

    def toggle_continuous_measurements(self):
        self.uart_handler.send_command("03")

    def get_data_from_continuous_mode(self) -> ContinuousModeInformation:
        received_msg: str = self.uart_handler.read_last_data()
        return self._process_data_from_continuous_mode(received_msg)

    @staticmethod
    def _process_data_from_continuous_mode(received_msg: str) -> ContinuousModeInformation:
        temperature_msg, pressure_msg, humidity_msg, empty = received_msg.split(";")
        temperature = float(temperature_msg.split(":")[1])
        pressure = float(pressure_msg.split(":")[1])
        humidity = float(humidity_msg.split(":")[1])
        return ContinuousModeInformation(temperature, pressure * 10, humidity)
