#ifndef MAIN_NODE_H
#define MAIN_NODE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <RF24.h>
#include <RF24Network.h>

const int MAX_STUDENT_NODES = 10; // 155
const int MAX_SENSOR_NODES = 1; // 5
const int NAME_LENGTH = 7;

struct Sensor_Node { // os inteiros têm tamanhos diferentes num e noutro
  char name[NAME_LENGTH];
  float temperature;
  float phototransistor;
};

struct Student_Node {
  uint16_t nodeID;
  char name[NAME_LENGTH];
};

struct Network_Status {
  Sensor_Node data;
  Student_Node connected_nodes[MAX_STUDENT_NODES];
  bool status = false;
  long time;
};


extern Network_Status network_status[MAX_SENSOR_NODES];

class MainNode {
  public:
    MainNode(int channel);
    void init();
    void setupMQTT(char* ssid, char* wifiPassword, char* server, int port);
    void receive24RFNetworkMessage();
    void connectPublisher(char* username, char* mqttPassword);
    void publishNetworkStatus(char* topic, const unsigned long interval);
    void checkNodesConnection(const unsigned long interval);
  
  private:
    int _channel;
    void setupRF24Network();
    void handle_R(RF24NetworkHeader& header);
    void handle_B(RF24NetworkHeader& header);
    void handle_S(RF24NetworkHeader& header);
    void handle_P(RF24NetworkHeader& header);
};

#endif