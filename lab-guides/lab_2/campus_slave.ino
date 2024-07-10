#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "CampusStudentNode.h"

uint16_t sensorNode = 01;
char name[NAME_LENGTH] = "BERN02";
int channel = 90;

CampusStudentNode studentNode(sensorNode, name, channel);

#define R_LED_PIN 3
#define Y_LED_PIN 4
#define G_LED_PIN 2

const int temperatureLimit = 19;
const double minBlinkRate = 0.2;
const double maxBlinkRate = 2.0;

unsigned long blinkInterval = 2000;
unsigned long previousMillisBlink = 0;
int ledState = LOW;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  studentNode.init();

  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(Y_LED_PIN, OUTPUT);
}

void loop()
{
  studentNode.performEssentialOperations();

  if (millis() - previousMillisBlink >= blinkInterval)
  {
    previousMillisBlink = millis();
    ledState = !ledState;
    digitalWrite(G_LED_PIN, ledState);
  }

  Message<int> msg = studentNode.receiveSimpleMessage<int>();
  if (msg.type == 'T')
  {
    studentNode.log(F(": Temperature received: "), msg.content);
    if (msg.content > temperatureLimit)
    {
      digitalWrite(R_LED_PIN, HIGH);
    }
    else
    {
      digitalWrite(R_LED_PIN, LOW);
    }
  }
  else if (msg.type == 'L')
  {
    studentNode.log(F(": Light received: "), msg.content);
    analogWrite(Y_LED_PIN, 255 - msg.content);
  }
  else if (msg.type == 'M')
  {
    studentNode.log(F(": Rotation received: "), msg.content);
    blinkInterval = map(msg.content, 0, 180, minBlinkRate * 1000, maxBlinkRate * 1000);
  }
}