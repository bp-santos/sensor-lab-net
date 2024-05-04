#include "student_node.h"
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

char name[NAME_LENGTH] = "BERN01";

StudentNode studentNode(01, name, 90); // (sensorNode, name, channel)

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
  studentNode.sendKeepAlive(3000);
  studentNode.restart();


}