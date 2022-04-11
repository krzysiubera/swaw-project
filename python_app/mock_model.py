import numpy as np


class MockModel:
    """ Class representing mock of the model of the application """

    @staticmethod
    def get_last_temperature() -> str:
        """ Get last temperature from UART """
        return f"{20}"

    @staticmethod
    def get_last_humidity() -> str:
        """ Get last humidity from UART """
        return f"{42}"

    @staticmethod
    def get_last_pressure():
        """ Get last pressure from UART """
        return f"{982}"

    @staticmethod
    def get_temperature_values() -> np.ndarray:
        """ Get measurements of temperature """
        min_val = 20
        max_val = 25
        return (max_val - min_val) * np.random.random(size=180) + min_val

    @staticmethod
    def get_humidity_values() -> np.ndarray:
        """ Get measurements of humidity """
        min_val = 40
        max_val = 60
        return (max_val - min_val) * np.random.random(size=180) + min_val

    @staticmethod
    def get_pressure_values() -> np.ndarray:
        """ Get measurements of pressure """
        min_val = 980
        max_val = 1000
        return (max_val - min_val) * np.random.random(size=180) + min_val
