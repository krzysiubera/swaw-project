import time
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
        """ Get temperature values from UART data buffer """
        self.uart_handler.send_command("get meas temp")
        return self._process_data_buffer()

    def get_pressure_values(self) -> np.ndarray:
        """ Get pressure values from UART data buffer """
        self.uart_handler.send_command("get meas press")
        return self._process_data_buffer()

    def get_humidity_values(self) -> np.ndarray:
        """ Get humidity values from UART data buffer """
        self.uart_handler.send_command("get meas hum")
        return self._process_data_buffer()

    def set_sampling_rate(self, sampling_rate_sec) -> None:
        """ Set sampling rate for BME280 measurements """
        self.uart_handler.send_command(f"set sampling rate:{sampling_rate_sec}")

    def _process_data_buffer(self) -> np.ndarray:
        """
        Receive measurements from UART data buffer until 'end_buffer_msg' is received or number of received measurements
        exceeds 'max_num_measurements'
        """
        collected_meas_num: int = 0
        measurements: np.ndarray = np.array([])
        while collected_meas_num < self.__class__.max_num_measurements:
            received_msg: str = self.uart_handler.read_last_data()
            if received_msg == self.__class__.end_buffer_msg:
                break
            measurements = np.append(measurements, float(received_msg))
            collected_meas_num = collected_meas_num + 1
        return measurements

    def get_sampling_rate(self) -> str:
        """
        Get information about current sampling rate (in seconds).
        :return string "%lu seconds", where %lu is a current sampling rate
        """
        self.uart_handler.send_command("get sampling rate")
        return self.uart_handler.read_last_data()


if __name__ == '__main__':
    model = Model()
    data_buffer_prev = model.get_temperature_values()
    print(f"Len data buffer prev: {len(data_buffer_prev)}")
    model.set_sampling_rate(5)
    time.sleep(60)
    data_buffer = model.get_temperature_values()
    print(f"Len data buffer now: {len(data_buffer)}")
