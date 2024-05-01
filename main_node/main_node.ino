#include "main_node.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>

MainNode mainNode(90, "NOS-0856", "FG94RWP5", "192.168.1.28", 1883, "Main Node"); // (channel, ssid, wifiPassword, server, port, topic)

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
  mainNode.receive24RFNetworkMessage();
  mainNode.checkNodesConnection(10000);
  mainNode.publishNetworkStatus(5000);
}