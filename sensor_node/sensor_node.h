#ifndef SENSOR_NODE_H
#define SENSOR_NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const int MAX_STUDENT_NODES = 124;
const int MAX_ALERT_PER_STUDENT = 1;
const int NAME_LENGTH = 7;

struct Sensor_Node {
  char name[NAME_LENGTH];
  int16_t temperature;
  int16_t phototransistor;
};

struct Student_Node {
  uint16_t nodeID;
  char name[NAME_LENGTH];
};

struct Alert_Request {
  char type = '\0';
  int value;
};

struct Active_Nodes {
  Student_Node node;
  Alert_Request alerts[MAX_ALERT_PER_STUDENT];
  bool status = false;
  long time;
};

extern Sensor_Node sensorData;
extern Active_Nodes active_nodes[MAX_STUDENT_NODES];

class SensorNode {
  public:
    SensorNode(uint16_t node, char* name, int channel);
    void init();
    void populateActiveNodesArray();
    void setupRF24Network();
    void updateSensorValues(int tempPin, int lightPin, const unsigned long interval);
    void fakeSensorValues(const unsigned long interval);
    void sendNetworkStatus(const unsigned long interval, uint16_t node);
    void checkAlerts();
    void checkNodesConnection(const unsigned long interval);
    void sendKeepAlive(const unsigned long interval, uint16_t to);
    void receive24RFNetworkMessage();

  private:
    uint16_t _node;
    int _channel;

    void handle_A(RF24NetworkHeader& header);
    void handle_D(RF24NetworkHeader& header);
    void handle_R(RF24NetworkHeader& header);
    void send_R(uint16_t to);
    uint16_t handle_N(RF24NetworkHeader& header);
    void send_N(uint16_t to, uint16_t id);
    void handle_P(RF24NetworkHeader& header);
    void send_B(uint16_t to);
    void send_S(uint16_t to);
};

#endif