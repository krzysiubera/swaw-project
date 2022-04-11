from PyQt5 import QtWidgets, uic
import pyqtgraph


class AppGui(QtWidgets.QMainWindow):
    """ Class responsible for GUI of the application """

    def __init__(self):
        """ Init GUI application and create elements on the screen """
        super().__init__()
        uic.loadUi('designer/app_gui.ui', self)

        self.temperature_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'temperature_button')
        self.temperature_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'temperature_label')
        self.humidity_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'humidity_button')
        self.humidity_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'humidity_label')
        self.pressure_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'pressure_button')
        self.pressure_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'pressure_label')

        self.temperature_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'temperature_plot')
        self.humidity_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'humidity_plot')
        self.pressure_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'pressure_plot')
        self.temperature_plot.showGrid(x=True, y=True, alpha=0.5)
        self.humidity_plot.showGrid(x=True, y=True, alpha=0.5)
        self.pressure_plot.showGrid(x=True, y=True, alpha=0.5)

        self.temperature_plot_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton,
                                                                             'temperature_plot_button')
        self.humidity_plot_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton,
                                                                          'humidity_plot_button')
        self.pressure_plot_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton,
                                                                          'pressure_plot_button')
