import serial
import csv
import time
import datetime

puerto_serial = serial.Serial('COM6', 9600, timeout=1)

data = [['Tiempo', 'Sensor']]
start = time.perf_counter()

try:
    while True:
        line = puerto_serial.readline().decode('utf-8').strip().split()
        line.insert(0, time.perf_counter()-start)
        data.append(line)

except:
    with open('cora.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(data)