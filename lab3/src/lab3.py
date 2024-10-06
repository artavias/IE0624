import serial

ser = serial.Serial("COM4", 9600)

with open('datos.csv', 'w') as file:
    file.write('V1,V2,V3,V4')
    file.write('\n')
    while True:
        try:
            v1 = ser.readline().decode().strip()
            v2 = ser.readline().decode().strip()
            v3 = ser.readline().decode().strip()
            v4 = ser.readline().decode().strip()
            line = "{},{},{},{}\n".format(v1, v2, v3, v4)
            file.write(line)
            print(line)
        except:
            print('Cerrando archivo')
            break

