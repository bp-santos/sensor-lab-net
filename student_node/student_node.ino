#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"
#include "CampusStudentNode.h"

// uint16_t sensorNode = 01;
uint16_t this_node = 00;
uint16_t other_node = 01;
char name[NAME_LENGTH] = "BERN02";
int channel = 90;

// CampusStudentNode studentNode(sensorNode, name, channel);
HomeStudentNode studentNode(this_node, name, channel);

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
  // studentNode.performEssentialOperations();
  // studentNode.sendReadingsRequestToSensorNode();

  // studentNode.log(F(": Message sent to "), other_node);
  studentNode.sendPayload(other_node, SIMPLE_MESSAGE, "Hello World!");
}