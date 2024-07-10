#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"

uint16_t this_node = 00;
uint16_t other_node = 01;
char name[NAME_LENGTH] = "BERN02";
int channel = 90;

HomeStudentNode studentNode(this_node, name, channel);

const int tempSensor = A0;
const int rotSensor = A3;
const int lightSensor = A1;

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
  if(millis() - previousMillis > interval) {
    handleLight();
    handleRotation();
    handleTemperature();
    previousMillis = millis();
    Serial.println();
  }
}

void handleLight() {
  int light = map(analogRead(lightSensor), 0, 1023, 0, 255);
  studentNode.log(F(": Light sent: "), light);
  studentNode.sendPayload(other_node, 'L', light);
}

void handleRotation() {
  int rotation = map(analogRead(rotSensor), 0, 1023, 0, 180);
  studentNode.log(F(": Rotation sent: "), rotation);
  studentNode.sendPayload(other_node, 'R', rotation);
}

void handleTemperature() {
  int temperature = (((analogRead(tempSensor) / 1023.0) * 5.0) - 0.5) * 100;
  studentNode.log(F(": Temperature sent: "), temperature);
  studentNode.sendPayload(other_node, 'T', temperature);
}