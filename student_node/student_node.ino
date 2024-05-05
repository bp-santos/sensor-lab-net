#include "student_node.h"
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

uint16_t sensorNode = 01;
char name[NAME_LENGTH] = "BERN01";
int channel = 90;

StudentNode studentNode(sensorNode, name, channel);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  studentNode.init();
}

void loop()
{
  studentNode.performEssentialOperations();
  studentNode.sendReadingsRequestToSensorNode();
  /*studentNode.sendMessage("BERN02", 'M', "Hello World");

  RF24NetworkHeader header = studentNode.receiveMessageHeader();
  if (header.type == 'M')
  {
    char message[32];
    studentNode.readMessage(header, message);

    Serial.print(F(": Received message from "));
    Serial.print(header.from_node);
    Serial.print(F(": "));
    Serial.println(message);
  }*/
}