import sys
from PyQt5 import QtWidgets
from app_gui import AppGui
from controller import Controller
from model import Model
from mock_model import MockModel


if __name__ == '__main__':
    app: QtWidgets.QApplication = QtWidgets.QApplication(sys.argv)
    view: AppGui = AppGui()
    model: MockModel = MockModel()
    controller = Controller(view, model)
    view.show()
    app.exec_()
