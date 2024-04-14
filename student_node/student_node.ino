#include "student_node.h"
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

StudentNode studentNode(01, 10570, 90); // (sensorNode, name, channel)

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  studentNode.init();
  // studentNode.sendAlertRequest('T', -100.0);
}

void loop() {
  studentNode.sendKeepAlive(3000);
  studentNode.receive24RFNetworkResponse();
  studentNode.sendReadingsRequest(5000);
}