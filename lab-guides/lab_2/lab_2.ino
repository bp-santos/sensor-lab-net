#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"
#include "CampusStudentNode.h"

#define YELLOW 4
#define RED 3
#define GREEN 2

uint16_t this_node = 00;
char name[NAME_LENGTH] = "BERN01";
int channel = 90;

CampusStudentNode studentNode(this_node, name, channel);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  studentNode.init();
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
}

void loop()
{
  studentNode.performEssentialOperations();
  studentNode.sendPayload(01, 'M', "Hello World");

  RF24NetworkHeader header = studentNode.receivePayload();
  if (header.type == 'M')
  {
    char message[32];
    studentNode.network.read(header, message, sizeof(message));

    Serial.print(F(": Received message from "));
    Serial.print(header.from_node);
    Serial.print(F(": "));
    Serial.println(message);
  }
}