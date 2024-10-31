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
    bateria = 0
    alarma = 0
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
        print(datos)

        datos = datos.split(": ")

        if (datos[0] == "Valor X"):
            valor_X = int(datos[1])
        if (datos[0] == "\rValor Y"):
            valor_Y = int(datos[1])
        if (datos[0] == "\rValor Z"):
            valor_Z = int(datos[1])
        if (datos[0] == "\rBateria"):
            bateria = float(datos[1])/1000
        if (bateria < 7500):
            alarma = 1
        if (bateria > 7500):
            alarma = 0
        data = {
            "nivelBateria": bateria,
            "valorX": valor_X,
            "valorY": valor_Y,
            "valorZ": valor_Z,
            "alarm": alarma
        }
        cliente.publish(TOPIC, json.dumps(data))

enviar_TB()
