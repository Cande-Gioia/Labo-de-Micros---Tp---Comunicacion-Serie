import threading
import time
from PyQt5 import QtWidgets, uic
from PyQt5.QtCore import QObject, pyqtSignal
from usb import usbReader

class MyObject(QObject):
    data_received = pyqtSignal(str)

class iniciar:
    def __init__(self):
        # init
        app = QtWidgets.QApplication([])
        self.ventana = uic.loadUi("ui.ui")
        self.ventana.show()

        self.num = 1
        self.info = 'R'


        self.grupos = [{'R':'', 'C':'', 'O':''}]*6

        self.lineEdits = [
            {'R':self.ventana.R0, 'C':self.ventana.C0, 'O':self.ventana.O7},
            {'R':self.ventana.R1, 'C':self.ventana.C1, 'O':self.ventana.O1},
            {'R':self.ventana.R2, 'C':self.ventana.C2, 'O':self.ventana.O2},
            {'R':self.ventana.R3, 'C':self.ventana.C3, 'O':self.ventana.O3},
            {'R':self.ventana.R4, 'C':self.ventana.C4, 'O':self.ventana.O4},
            {'R':self.ventana.R5, 'C':self.ventana.C5, 'O':self.ventana.O5},
            {'R':self.ventana.R6, 'C':self.ventana.C6, 'O':self.ventana.O6}
        ]


        self.my_object = MyObject()


        self.my_object.data_received.connect(self.updateLineEdit)

        self.reader = usbReader(baudrate=460800)

        self.usb_thread = threading.Thread(target=self.usb_data_reception)
        self.usb_thread.daemon = True 
        self.usb_thread.start()



        app.exec()


    def updateLineEdit(self, new_value):
            self.lineEdits[self.num][self.info].setText(new_value)


    def usb_data_reception(self):
        while True:
            data = self.reader.readUSB()
            if len(data) > 3:
                self.num = int(data[0])
                self.info = data[1]

                newData = data[2:]

                self.my_object.data_received.emit(newData)
