import serial
import paho.mqtt.client as mqtt

ser = serial.Serial('COM3', 115200) # Change COM3 to the port your device is connected to

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("Lab 1") # Change "Lab 1" to the topic you want to subscribe to

def on_message(client, userdata, msg):
    print(f"{msg.topic} {msg.payload}")
    ser.write(msg.payload)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.1.1", 1883, 60) # Change the IP address to the IP address of your MQTT broker
client.loop_start()

while True:
    pass