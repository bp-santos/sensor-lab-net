#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"

uint16_t this_node = 00;
uint16_t other_node = 01;
char name[NAME_LENGTH] = "TEST00";
int channel = 90;

HomeStudentNode studentNode(this_node, name, channel);

long sent = 0.0;
long success = 0.0;
bool f = false;
byte payload[12] = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  studentNode.init();
}

void loop() {
  if (sent < 1000) {
    bool ok = studentNode.sendPayload(other_node, 'T', payload);
    if (ok) success++;
    sent++;
  } else {
    if (!f) {
      studentNode.sendPayload(other_node,'F',0);
      studentNode.log(F(" : Sent - "), sent);
      studentNode.log(F(" : Retransmitted - "), sent - success);
      f = true;
    }
  }
}