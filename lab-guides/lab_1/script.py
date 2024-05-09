import serial
import paho.mqtt.client as mqtt

# Create a serial connection
ser = serial.Serial('COM3', 115200)

# Define MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("Lab 1")

def on_message(client, userdata, msg):
    print(f"{msg.topic} {msg.payload}")
    ser.write(msg.payload)

# Create an MQTT client and assign the callbacks
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker and start the loop
client.connect("192.168.1.86", 1883, 60)
client.loop_start()

# Keep the script running
while True:
    pass