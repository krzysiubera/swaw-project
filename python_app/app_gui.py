from PyQt5 import QtWidgets, uic
import pyqtgraph


class AppGui(QtWidgets.QMainWindow):
    """ Class responsible for GUI of the application """

    def __init__(self):
        """ Init GUI application and create elements on the screen """
        super().__init__()
        uic.loadUi('app_gui.ui', self)

        self.temperature_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'temperature_button')
        self.temperature_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'temperature_label')
        self.humidity_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'humidity_button')
        self.humidity_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'humidity_label')
        self.pressure_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'pressure_button')
        self.pressure_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'pressure_label')

        # demo for pyqtgraph - to be removed
        self.graph = self.findChild(pyqtgraph.PlotWidget, 'graph_widget')
        x_axis = [x for x in range(10)]
        y_axis = [1 for _ in range(10)]
        self.graph.plot(x_axis, y_axis)
