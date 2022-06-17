from PyQt5 import QtWidgets, QtCore, uic
import pyqtgraph


class AppGui(QtWidgets.QMainWindow):
    """ Class responsible for GUI of the application """

    def __init__(self):
        """ Init GUI application and create elements on the screen """
        super().__init__()
        uic.loadUi('designer/app_gui.ui', self)

        # for switching between tabs
        self.tab_widget: QtWidgets.QTabWidget = self.findChild(QtWidgets.QTabWidget, "tabWidget")
        self.settings_widget: QtWidgets.QWidget = self.findChild(QtWidgets.QWidget, "settings_widget")
        self.list_files_widget: QtWidgets.QWidget = self.findChild(QtWidgets.QWidget, "list_files_widget")
        self.last_measurements_widget: QtWidgets.QWidget = self.findChild(QtWidgets.QWidget, "last_measurements_widget")
        self.plot_widget: QtWidgets.QWidget = self.findChild(QtWidgets.QWidget, "plot_widget")

        # for showing last measurements (continuous update)
        self.temperature_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'temperature_label')
        self.humidity_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'humidity_label')
        self.pressure_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, 'pressure_label')
        self.last_update_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, "last_update_label")

        # for plotting
        self.file_name_line_edit: QtWidgets.QLineEdit = self.findChild(QtWidgets.QLineEdit, "file_name_line_edit")
        self.plot_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, 'plot_button')
        self.datetime_info_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, "datetime_info_label")

        self.temperature_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'temperature_plot')
        self.humidity_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'humidity_plot')
        self.pressure_plot: pyqtgraph.PlotWidget = self.findChild(pyqtgraph.PlotWidget, 'pressure_plot')
        self.temperature_plot.showGrid(x=True, y=True, alpha=0.5)
        self.humidity_plot.showGrid(x=True, y=True, alpha=0.5)
        self.pressure_plot.showGrid(x=True, y=True, alpha=0.5)

        # for setting sampling time and continuous update of timer
        self.sampling_rate_line_edit: QtWidgets.QLineEdit = self.findChild(QtWidgets.QLineEdit,
                                                                           "sampling_rate_line_edit")
        self.start_measurements_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton,
                                                                               "start_measurements_button")
        self.sampling_rate_info_label: QtWidgets.QLabel = self.findChild(QtWidgets.QLabel, "sampling_rate_info_label")
        self.timer_last_measurements: QtCore.QTimer = QtCore.QTimer()

        # for listing files
        self.list_files_button: QtWidgets.QPushButton = self.findChild(QtWidgets.QPushButton, "list_files_button")
        self.list_files_text_browser: QtWidgets.QTextBrowser = self.findChild(QtWidgets.QTextBrowser,
                                                                              "list_files_text_browser")

    @staticmethod
    def show_error_message(message: str) -> None:
        """
        Create an error message and show it on the screen
        :param message: text to be put in the error message
        """
        msg = QtWidgets.QMessageBox()
        msg.setIcon(QtWidgets.QMessageBox.Critical)
        msg.setText("Error")
        msg.setInformativeText(message)
        msg.setWindowTitle("Error")
        msg.exec_()
