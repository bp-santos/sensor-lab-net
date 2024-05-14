#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include "PreInstalledNode.h"

const int MAX_ALERT_PER_STUDENT = 1;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const long unsigned SENSOR_DATA_UPDATE_INTERVAL = 5000;

const char SELF_ID_REQUEST = 'N';
const char ID_REQUEST = 'I';
const char ALERT_REQUEST = 'A';
const char ALERT_DEACTIVATION = 'D';

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

class SensorNode : public PreInstalledNode
{
public:
  SensorNode(uint16_t node, char *name, int channel, uint16_t mainNode);

  void init() override;
  void receivePayload() override;
  void checkNodesConnection() override;

  void sendKeepAlive();
  void updateSensorValues(int tempPin, int lightPin);
  void generateRandomSensorValues();
  void sendNetworkStatus();
  void checkAlerts();

private:
  Sensor_Node sensorData;
  Active_Nodes active_nodes[MAX_STUDENT_NODES];
  uint16_t _mainNode;

  unsigned long last_reading;
  unsigned long last_status_sent;
  unsigned long last_sent_keep_alive;

  void receiveKeepAlive(RF24NetworkHeader &header) override;

  void populateActiveNodesArray();
  int octalToDecimal(uint16_t octalNumber);
  uint16_t receiveNodeIDRequest(RF24NetworkHeader &header);
  void sendNextAvailableNodeID(uint16_t to, uint16_t id);
  uint16_t receiveNodeIDRequestFromName(RF24NetworkHeader &header);
  void sendNodeID(uint16_t to, uint16_t id);
  void receiveAlertRequest(RF24NetworkHeader &header);
  void receiveAlertDeactivationRequest(RF24NetworkHeader &header);
  void receiveReadingsRequest(RF24NetworkHeader &header);
  void sendReadings(uint16_t to);
  void sendBeginFlagArray();
  void sendArrayOfActiveNodes();
};

#endif