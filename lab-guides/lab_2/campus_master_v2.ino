#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "CampusStudentNode.h"

uint16_t sensorNode = 01;
char name[NAME_LENGTH] = "BERN01";
char other_node_name[NAME_LENGTH] = "BERN02";
int channel = 90;

CampusStudentNode studentNode(sensorNode, name, channel);

const int rotSensor = A3;

const unsigned long interval = 2000;
unsigned long previousMillis = 0;

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

  if (millis() - previousMillis > interval)
  {
    studentNode.sendReadingsRequestToSensorNode();
    Sensor_Node temp = studentNode.receiveReadingsFromSensorNode();

    handleLight(temp.phototransistor);
    handleTemperature(temp.temperature);
    handleRotation();
    
    previousMillis = millis();
    Serial.println();
  }
}

void handleLight(int light)
{
  studentNode.log(F(": Light sent: "), light);
  studentNode.sendMessage(other_node_name, 'L', light);
}

void handleRotation()
{
  int rotation = map(analogRead(rotSensor), 0, 1023, 0, 180);
  studentNode.log(F(": Rotation sent: "), rotation);
  studentNode.sendMessage(other_node_name, 'M', rotation);
}

void handleTemperature(int temperature)
{
  studentNode.log(F(": Temperature sent: "), temperature);
  studentNode.sendMessage(other_node_name, 'T', temperature);
}