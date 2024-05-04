#ifndef STUDENT_NODE_H
#define STUDENT_NODE_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

const uint16_t NODE_BASE = 010;
const uint16_t SENSOR_NODE_MIN = 1;
const uint16_t SENSOR_NODE_MAX = 5;
const int RF24_PA_LEVEL = RF24_PA_HIGH;
const int RADIO_CE_PIN = 7;
const int RADIO_CSN_PIN = 8;
const int NAME_LENGTH = 7;
const int MAX_FAILED_MESSAGES = 5;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const unsigned long INIT_DELAY = 2000;
const unsigned long ID_REQUEST_DELAY = 5000;
const unsigned long PAYLOAD_SEND_DELAY = 100;

const char SELF_ID_REQUEST = 'N';
const char ID_REQUEST = 'I';
const char ALERT_REQUEST = 'A';
const char READINGS_REQUEST = 'R';

const char KEEP_ALIVE = 'P';
const char ALERT_DEACTIVATION = 'D';

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
  void performEssentialOperations();
   
  void sendReadingsRequestToSensorNode();
  void sendAlertRequestToSensorNode(char type, int value);
  void sendAlertDeactivationToSensorNode();

  void sendMessage(char *name, char type, const void *message);
  RF24NetworkHeader receiveMessageHeader();
  void readMessage(RF24NetworkHeader header, void *message);

private:
  RF24 radio;           //  nRF24L01(+) radio attached using Getting Started board
  RF24Network network;

  unsigned long last_sent_reading;            // When did we send the last readings request?
  unsigned long last_sent_alert_deactivation; // When did send the last alert deactivation?
  unsigned long last_sent_keep_alive;         // When did we send the last keep alive?

  int countFailedMessages = 0;

  uint16_t nodeID;

  uint16_t _sensorNode;
  uint16_t _node;
  char _name[NAME_LENGTH];
  int _channel;

  void setupRF24Network();
  void sendPayload(uint16_t to, char type, const void *payload);
  void receivePayload();
  
  void sendIDRequest(char type, const char *name);
  void sendKeepAlive(const unsigned long interval);
  void restart();

  template<typename... Args>
  void log(Args... args);
};

#endif