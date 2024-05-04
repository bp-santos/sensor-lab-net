#include "main_node.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>

uint16_t node = 00;
int channel = 90;
char *ssid = "NOS-0856";
char *wifiPassword = "FG94RWP5";
char *server = "192.168.1.28";
short port = 1883;
char *topic = "Main Node";

MainNode mainNode(node, channel, ssid, wifiPassword, server, port, topic);

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