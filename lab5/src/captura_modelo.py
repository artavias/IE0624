import serial
import csv
import time
import datetime

puerto_serial = serial.Serial('COM6', 115200, timeout=1)
data = []
try:
    while(True):
        line = puerto_serial.readline().decode('utf-8')
        data.append(line)

except:
    with open('resultado_modelo.txt', 'w', newline='\n') as file:
        for line in data:
            file.write(str(line))