import sys
from PyQt5 import QtWidgets
from app_gui import AppGui
from controller import Controller


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    view = AppGui()
    controller = Controller(view)
    view.set_controller(controller)
    view.show()
    app.exec_()
