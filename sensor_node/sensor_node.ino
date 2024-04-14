#include "sensor_node.h"
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

SensorNode sensorNode(01, 90); // (node, channel)
int tempPin = 1;
int lightPin = 2;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  sensorNode.init();
}

void loop() {
  sensorNode.sendKeepAlive(3000, 00);
  sensorNode.receive24RFNetworkMessage();
  //sensorNode.updateSensorValues(tempPin, lightPin, 5000);
  sensorNode.fakeSensorValues(5000);
  sensorNode.checkNodesConnection(10000);
  sensorNode.sendNetworkStatus(5000, 00);
  sensorNode.checkAlerts();
}