import serial
import csv
import time
import datetime

puerto_serial = serial.Serial('COM6', 9600, timeout=1)

labels = ['circulos', 'golpe', 'estacionario']
for label in labels:
    print(label)
    time.sleep(2)
    print('starting')
    for i in range(4):
        data = [['time','gyrX', 'gyrY', 'gyrZ', 'label']]
        print(label+str(i)+'.csv')
        start = time.perf_counter()
        while time.perf_counter() - start < 10:
            line = puerto_serial.readline().decode('utf-8').strip().split()
            line.append(label)
            line.insert(0, time.perf_counter()-start)
            data.append(line)

        with open('TEST_'+label+str(i)+'.csv', 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerows(data)