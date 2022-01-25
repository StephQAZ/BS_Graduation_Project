import random
import time

from paho.mqtt import client as mqtt_client

broker = "192.168.31.125"
port = 8883
topic = "t1"
client_id = "2"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code is %d\n", rc)

def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

if __name__ == '__main__':
    client = mqtt_client.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect('192.168.31.125', 8883, 60)
    client.publish('FIFA', payload='hello', qos=0)


                

