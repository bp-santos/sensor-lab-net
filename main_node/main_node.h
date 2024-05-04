#ifndef MAIN_NODE_H
#define MAIN_NODE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>
#include <ArduinoJson.h>

const int RADIO_CE_PIN = 7;
const int RADIO_CSN_PIN = 8;
const int RF24_PA_LEVEL = RF24_PA_HIGH;
const int MAX_STUDENT_NODES = 124;
const int MAX_SENSOR_NODES = 1;
const int NAME_LENGTH = 7;
const unsigned long INIT_DELAY = 2000;
const unsigned long WIFI_CONNECT_DELAY = 5000;
const unsigned long MQTT_CONNECT_DELAY = 5000;
const unsigned long NODE_CONNECTION_CHECK_INTERVAL = 10000;
const unsigned long NETWORK_STATUS_SEND_INTERVAL = 10000;

const char KEEP_ALIVE = 'P';
const char READINGS_REQUEST = 'R';
const char BEGIN_FLAG = 'B';
const char ACTIVE_NODES = 'S';

struct Sensor_Node
{
  char name[NAME_LENGTH];
  int16_t temperature;
  int16_t phototransistor;
};

struct Student_Node
{
  uint16_t nodeID;
  char name[NAME_LENGTH];
};

struct Network_Status
{
  Sensor_Node data;
  Student_Node connected_nodes[MAX_STUDENT_NODES];
  bool status = false;
  long time;
};

class MainNode
{
public:
  MainNode(uint16_t node, int channel, char *ssid, char *wifiPassword, char *server, short port, char *topic);
  void init();
  void checkMQTTConnection();
  void receivePayload();
  void checkNodesConnection();
  void publishNetworkStatus();

private:
  Network_Status network_status[MAX_SENSOR_NODES];
  RF24 radio;
  RF24Network network;
  WiFiClient espClient;
  PubSubClient client;

  unsigned long last_sent;

  uint16_t _node;
  int _channel;
  char *_ssid;
  char *_wifiPassword;
  char *_server;
  short _port;
  char *_topic;

  void setupWiFi();
  void setupMQTT();
  void setupRF24Network();
  void receiveKeepAlive(RF24NetworkHeader &header);
  void receiveReadings(RF24NetworkHeader &header);
  void receiveBeginFlag(RF24NetworkHeader &header);
  void receiveNodeID(RF24NetworkHeader &header);

  template <typename... Args>
  void log(Args... args);
};

#endif