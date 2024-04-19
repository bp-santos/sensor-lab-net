#ifndef STUDENT_NODE_H
#define STUDENT_NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const int NAME_LENGTH = 7;

struct Sensor_Node
{
  char name[NAME_LENGTH];
  int16_t temperature;
  int16_t phototransistor;
};

struct Alert_Request
{
  char type = '\0';
  int value;
};

class StudentNode
{
public:
  StudentNode(uint16_t sensorNode, char *name, int channel);
  void init();
  void receive24RFNetworkResponse();
  void sendReadingsRequest(const unsigned long interval);
  void sendAlertDeactivation(const unsigned long interval);
  void sendAlertRequest(char type, int value);
  void sendKeepAlive(const unsigned long interval);
  void restart();

private:
  uint16_t _sensorNode;
  uint16_t _node;
  char _name[NAME_LENGTH];
  int _channel;

  void setupRF24Network();
  void sendIDRequest();
  void handle_N(RF24NetworkHeader &header);
  void handle_R(RF24NetworkHeader &header);
  void handle_A(RF24NetworkHeader &header);
  void handle_C(RF24NetworkHeader &header);
};

#endif