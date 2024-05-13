#include "SensorNode.h"
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

uint16_t node = 01;
char name[NAME_LENGTH] = "NODE01";
int channel = 90;
uint16_t mainNode = 00;

SensorNode sensorNode(node, name, channel, mainNode);

int tempPin = 1;
int lightPin = 2;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  sensorNode.init();
}

void loop()
{
  sensorNode.sendKeepAlive();
  sensorNode.receivePayload();
  // sensorNode.updateSensorValues(tempPin, lightPin);
  sensorNode.generateRandomSensorValues();
  sensorNode.checkNodesConnection();
  sensorNode.sendNetworkStatus();
  sensorNode.checkAlerts();
}