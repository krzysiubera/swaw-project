import sys
from PyQt5 import QtWidgets
from app_gui import AppGui
from controller import Controller
from model import Model


if __name__ == '__main__':
    app: QtWidgets.QApplication = QtWidgets.QApplication(sys.argv)
    view: AppGui = AppGui()
    model: Model = Model()
    controller = Controller(view, model)
    controller.fill_default_values()
    controller.connect_signals_to_slots()
    controller.init_timer()
    view.show()
    app.exec_()
