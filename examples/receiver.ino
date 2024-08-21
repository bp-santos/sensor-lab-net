#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"

uint16_t this_node = 01;
char name[NAME_LENGTH] = "TEST01";
int channel = 90;

HomeStudentNode studentNode(this_node, name, channel);

long count = 0.0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  studentNode.init();
}

void loop() {
  Message<byte[12]> msg = studentNode.receiveSimpleMessage<byte[12]>();
  if (msg.type == 'T') {
    count++;
  }
  if(msg.type == 'F'){
    studentNode.log(F(" : Received - "), count);
    count=0;
  }
}