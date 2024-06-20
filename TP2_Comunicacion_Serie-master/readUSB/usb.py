import serial
import time
import platform


class usbReader:
    def __init__(self, baudrate=460800):
        self.buffer = ""

        if platform.system() == 'Linux':
            x = '/dev/ttyACM0'
        elif platform.system() == 'Windows':
            x = 'COM3'

        self.ser = serial.Serial(x, baudrate=baudrate)

    def __del__(self):
        self.ser.close() 

    def readUSB(self):

        char = ''

        #start_time = time.time()
        #timeout_seconds = 2

        while char != 'R' and char != 'O' and char != 'C' or not len(self.buffer):
            char = self.ser.read().decode('utf-8')
            self.buffer += char
            #print(time.time()- start_time)
            '''
            if time.time() - start_time >= timeout_seconds:
                data = self.buffer
                self.buffer = ""
                return data
            '''
        data = self.buffer[:-2]
        self.buffer = self.buffer[-2:]

        return data




