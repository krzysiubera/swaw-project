import numpy as np
from uart_handler import UartHandler


class Model:
    """ Class representing model of the application """
    max_num_measurements: int = 180
    end_buffer_msg: str = "End buffer"

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

    def get_last_pressure(self) -> str:
        """ Get last pressure from UART """
        self.uart_handler.send_command("get press")
        return self.uart_handler.read_last_data()

    def get_temperature_values(self) -> np.ndarray:
        self.uart_handler.send_command("get meas temp")
        return self._process_data_buffer()

    def get_pressure_values(self) -> np.ndarray:
        self.uart_handler.send_command("get meas press")
        return self._process_data_buffer()

    def get_humidity_values(self) -> np.ndarray:
        self.uart_handler.send_command("get meas hum")
        return self._process_data_buffer()

    def _process_data_buffer(self) -> np.ndarray:
        collected_meas_num: int = 0
        measurements: np.ndarray = np.array([])
        while collected_meas_num < self.__class__.max_num_measurements:
            received_msg: str = self.uart_handler.read_last_data()
            if received_msg == self.__class__.end_buffer_msg:
                break
            measurements = np.append(measurements, float(received_msg))
            collected_meas_num = collected_meas_num + 1
        return measurements
