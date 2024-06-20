
import serial
ser = serial.Serial('COM5')  # open serial port
print(ser.name)         # check which port was really used
#ser.read(5)     # write a string
print(ser.read(9))
ser.close()    