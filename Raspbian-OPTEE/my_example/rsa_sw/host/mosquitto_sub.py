import os
import sys
import datetime
import socket, sys
import ssl
import paho.mqtt.client as mqtt_client
import time

# server address
broker = "192.168.31.125"

# commuication port
port = 8883

# topic
topic = "t1"

# client ID
client_id = "1"

# cert path
my_cafile = "/etc/mosquitto/certs/ca.crt"
my_certfile = "/etc/mosquitto/certs/server.crt"
my_keyfile = "/etc/mosquitto/certs/server.key"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)
            
def on_message(client, userdata, msg):
    print("/********************/")
    encrypt_data = msg.payload.decode()
    #print(encrypt_data)
    os.system("sudo ./optee_example_rsa rsa2048 dec " + "\"" + encrypt_data + "\"")

if __name__ == '__main__':
    client = mqtt_client.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.tls_set(ca_certs=my_cafile, certfile=None, keyfile=None, cert_reqs=ssl.CERT_REQUIRED,
                        tls_version=ssl.PROTOCOL_TLSv1_2, ciphers=None)
    client.tls_insecure_set(True)
    client.connect('192.168.31.125', 8883, 60)
    client.subscribe('t1', qos=0)
    client.loop_forever()


