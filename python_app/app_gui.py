from PyQt5 import QtWidgets, uic


class AppGui(QtWidgets.QMainWindow):

    def __init__(self):
        super().__init__()
        uic.loadUi('app_gui.ui', self)

        self.temperature_button = self.findChild(QtWidgets.QPushButton, 'pushButton')
        self.temperature_label = self.findChild(QtWidgets.QLabel, 'label')
        self.controller = None

    def set_controller(self, controller):
        self.controller = controller
        self.temperature_button.clicked.connect(self.controller.handle_request_temperature)
