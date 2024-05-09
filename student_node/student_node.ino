#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"

uint16_t sensorNode = 01;
char name[NAME_LENGTH] = "BERN01";
int channel = 90;

// CampusStudentNode studentNode(sensorNode, name, channel);
HomeStudentNode studentNode(sensorNode, name, channel);

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
  //studentNode.performEssentialOperations();
  //studentNode.sendReadingsRequestToSensorNode();
}