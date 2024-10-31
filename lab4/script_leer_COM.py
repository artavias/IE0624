import serial
import paho.mqtt.client as mqtt
import json
import threading

# Se debe configurar dependiendo de cada puerto
puerto_serial = serial.Serial('COM14', 115200, timeout=1)

datos = 0


""" Este codigo se puede usar para guardar
datos si se requiere un registro
with open('reg_datos.csv', mode='a', newline='') as reg_datos:

writer = csv.writer(reg_datos)
 writer.writerow(datos)
 """


def enviar_TB():
    valor_X = 0
    valor_Y = 0
    valor_Z = 0
    token = "eiCEzMR90IHO0grSHBia"
    host = "iot.eie.ucr.ac.cr"
    puerto = 1883
    TOPIC = "v1/devices/me/telemetry"
    cliente = mqtt.Client()
    cliente.username_pw_set(token)
    cliente.connect(host, puerto, 60)
    cliente.loop_start()
    while True:
        datos = puerto_serial.readline().decode('utf-8').rstrip()

        datos = datos.split(": ")
        print(datos)
        print("separador")

        if (datos[0] == "Valor X"):
            valor_X = int(datos[1])
            print(datos[1])
        if (datos[0] == "\rValor Y"):
            valor_Y = int(datos[1])
            print(datos[1])
        if (datos[0] == "\rValor Z"):
            valor_Z = int(datos[1])
            print(datos[1])
        data = {
            "nivelBateria": 60,
            "humidity": 20,
            "valorX": valor_X,
            "valorY": valor_Y,
            "valorZ": valor_Z
        }
        cliente.publish(TOPIC, json.dumps(data))
        # print(datos[1] + "\n")
        # cliente.loop_start()


enviar_TB()
