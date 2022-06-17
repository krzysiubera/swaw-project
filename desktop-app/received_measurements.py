from dataclasses import dataclass
import numpy as np


@dataclass
class PlotInformation:
    x_axis: np.ndarray
    temperature: np.ndarray
    pressure: np.ndarray
    humidity: np.ndarray


@dataclass
class ContinuousModeInformation:
    temperature: float
    pressure: float
    humidity: float
