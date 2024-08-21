#ifndef MAIN_NODE_H
#define MAIN_NODE_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "PreInstalledNode.h"

const int MAX_SENSOR_NODES = 1;
const unsigned long WIFI_CONNECT_DELAY = 5000;
const unsigned long MQTT_CONNECT_DELAY = 5000;

struct Network_Status
{
  Sensor_Node data;
  Student_Node connected_nodes[MAX_STUDENT_NODES];
  bool status = false;
  long time;
};

class MainNode : public PreInstalledNode
{
public:
  MainNode(uint16_t node, int channel);

  void init() override;
  void checkMQTTConnection();
  void receivePayload() override;
  void checkNodesConnection() override;

  void publishNetworkStatus();

private:
  Network_Status network_status[MAX_SENSOR_NODES];
  WiFiClient espClient;
  PubSubClient client;

  unsigned long last_sent;

  char *_ssid;
  char *_wifiPassword;
  char *_server;
  short _port;
  char *_topic;

  void receiveKeepAlive(RF24NetworkHeader &header) override;
  void setupWiFi();
  void setupMQTT();
  void receiveReadings(RF24NetworkHeader &header);
  Sensor_Node deserializeSensorNode(uint8_t* buffer);
  void receiveBeginFlag(RF24NetworkHeader &header);
  void receiveNodeID(RF24NetworkHeader &header);
  int decimalToOctal(uint16_t octalNumber);
};

#endif