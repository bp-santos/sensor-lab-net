#include "main_node.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>

char *ssid = "Bernardo";
char *wifiPassword = "12345678";
char *server = "b37.mqtt.one";
short port = 1883;
char *username = "29adpt5613";
char *mqttPassword = "178ahilorw";
char *topic = "29adpt5613/home";

MainNode mainNode(90); // (channel)
const unsigned long interval = 5000;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  mainNode.init();
  // mainNode.setupMQTT(ssid, wifiPassword, server, port);
}

void loop()
{
  mainNode.receive24RFNetworkMessage();
  mainNode.checkNodesConnection(10000);
  // mainNode.connectPublisher(username, mqttPassword);
  mainNode.publishNetworkStatus(topic, interval);
}