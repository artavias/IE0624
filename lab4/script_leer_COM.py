import serial
import csv

# Se debe configurar dependiendo de cada puerto
puerto_serial = serial.Serial('COM14', 115200, timeout=1)

while True:
    datos = puerto_serial.readline().decode('utf-8').rstrip()
    print(datos + "\n")

    """ Este codigo se puede usar para guardar datos si se requiere un registro
    with open('reg_datos.csv', mode='a', newline='') as reg_datos:

        writer = csv.writer(reg_datos)
        writer.writerow(datos)
    """
