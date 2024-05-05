#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const int RADIO_CE_PIN = 7;
const int RADIO_CSN_PIN = 8;
const int RF24_PA_LEVEL = RF24_PA_HIGH;
const int MAX_STUDENT_NODES = 124;
const int MAX_ALERT_PER_STUDENT = 1;
const int NAME_LENGTH = 7;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const long unsigned INIT_DELAY = 2000;
const long unsigned SENSOR_DATA_UPDATE_INTERVAL = 5000;
const long unsigned NETWORK_STATUS_SEND_INTERVAL = 10000;
const long unsigned NODE_CONNECTION_CHECK_INTERVAL = 10000;
const long unsigned PAYLOAD_SEND_DELAY = 100;

const char SELF_ID_REQUEST = 'N';
const char ID_REQUEST = 'I';
const char ALERT_REQUEST = 'A';
const char READINGS_REQUEST = 'R';
const char KEEP_ALIVE = 'P';
const char ALERT_DEACTIVATION = 'D';
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

struct Alert_Request
{
  char type = '\0';
  int value;
};

struct Active_Nodes
{
  Student_Node node;
  Alert_Request alerts[MAX_ALERT_PER_STUDENT];
  bool status = false;
  long time;
};

class SensorNode
{
public:
  SensorNode(uint16_t node, char *name, int channel, uint16_t masterNode);
  void init();
  void sendKeepAlive();
  void receivePayload();
  void updateSensorValues(int tempPin, int lightPin);
  void generateRandomSensorValues();
  void checkNodesConnection();
  void sendNetworkStatus();
  void checkAlerts();

private:
  RF24 radio;
  RF24Network network;

  unsigned long last_reading;
  unsigned long last_status_sent;
  unsigned long last_sent_keep_alive;

  Sensor_Node sensorData;
  Active_Nodes active_nodes[MAX_STUDENT_NODES];

  uint16_t _node;
  uint16_t _mainNode;
  int _channel;

  void populateActiveNodesArray();
  int octalToDecimal(uint16_t octalNumber);
  void setupRF24Network();

  template <typename T>
  bool sendPayload(uint16_t to, char type, const T &payload);

  uint16_t receiveNodeIDRequest(RF24NetworkHeader &header);
  void sendNextAvailableNodeID(uint16_t to, uint16_t id);
  uint16_t receiveNodeIDRequestFromName(RF24NetworkHeader &header);
  void sendNodeID(uint16_t to, uint16_t id);
  void receiveAlertRequest(RF24NetworkHeader &header);
  void receiveAlertDeactivationRequest(RF24NetworkHeader &header);
  void receiveReadingsRequest(RF24NetworkHeader &header);
  void sendReadings(uint16_t to);
  void receiveKeepAlive(RF24NetworkHeader &header);

  void sendBeginFlagArray();
  void sendArrayOfActiveNodes();

  template <typename... Args>
  void log(Args... args);
};

#endif