import sys
from PyQt5 import QtWidgets
from app_gui import AppGui


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    view = AppGui()
    view.show()
    app.exec_()
