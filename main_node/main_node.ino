#include "MainNode.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>

uint16_t node = 00;
int channel = 90;

MainNode mainNode(node, channel);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  mainNode.init();
}

void loop()
{
  mainNode.checkMQTTConnection();
  mainNode.receivePayload();
  mainNode.checkNodesConnection();
  mainNode.publishNetworkStatus();
}