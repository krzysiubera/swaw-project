from PyQt5 import QtWidgets, uic
from uart_handler import UartHandler


class AppGui(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()
        uic.loadUi('app_gui.ui', self)
        self.uart_handler = UartHandler()

        self.temperature_button = self.findChild(QtWidgets.QPushButton, 'pushButton')
        self.temperature_button.clicked.connect(self.put_text_to_temperature_label)

        self.temperature_label = self.findChild(QtWidgets.QLabel, 'label')

    def put_text_to_temperature_label(self, text):
        self.uart_handler.send_command("get temp")
        result = self.uart_handler.read_last_data()
        self.temperature_label.setText(f"{result} deg C")
