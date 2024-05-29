#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "CampusStudentNode.h"

uint16_t sensorNode = 01;
char name[NAME_LENGTH] = "BERN02";
int channel = 90;

CampusStudentNode studentNode(sensorNode, name, channel);

char controller_name[NAME_LENGTH] = "BERN01";

const int RED = 0;
const int GREEN = 1;
const int OFF = 2;
const int PING = 3;
const int ACK = 4;
const int STATUS = 5;

#define NUMBER_OF_TRAFFIC_LIGHTS 8
#define TL_AR 10
#define TL_AY 9
#define TL_AG 8
#define PL_AR 7
#define PL_AG 6 // Simplification: Implement only the green pedestrian signal:
// green ON means "safe to cross the street", green OFF "do not cross, or take care to cross".
#define PEDESTRIAN_BUTTON 5
#define TL_BR 13
#define TL_BY 12
#define TL_BG 11
int lights[NUMBER_OF_TRAFFIC_LIGHTS] = {TL_AR, TL_AY, TL_AG, PL_AR, PL_AG, TL_BR, TL_BY, TL_BG};

const int yellowInterval = 500;
unsigned long previousMillisYellowBlink = 0;
int yellowBlinkInterval = 1000;
bool yellowLedState = LOW;
bool blinkYellow = false;

int pedestRedFailing = 0;
int pedestYellowFailing = 0;
int pedestGreenFailing = 0;
int redFailing = 0;
int yellowFailing = 0;
int greenFailing = 0;
int timerActivated = 0;

int lastPedestrianButtonState;
int currentPedestrianButtonState;

char ack[4];
char status[5];

bool msgReceived = false;
int msgType = -1;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  studentNode.init();

  // Initialises the traffic lights as outputs
  for (int i = 0; i < NUMBER_OF_TRAFFIC_LIGHTS; i++)
  {
    pinMode(lights[i], OUTPUT);
  }

  // Initialises the pedestrian button as input
  pinMode(PEDESTRIAN_BUTTON, INPUT);
}

void loop()
{
  checkPedestrianButton();
  if (blinkYellow)
  {
    handleYellowBlink();
  }
  receiveMessage();
}

void checkPedestrianButton()
{
  lastPedestrianButtonState = currentPedestrianButtonState;
  currentPedestrianButtonState = digitalRead(PEDESTRIAN_BUTTON);
  if (lastPedestrianButtonState == HIGH && currentPedestrianButtonState == LOW)
  {
    timerActivated = true;
  }
}

void handleYellowBlink()
{
  if (millis() - previousMillisYellowBlink > yellowBlinkInterval)
  {
    if (yellowLedState == LOW)
    {
      yellowLedState == HIGH;
    }
    else
    {
      yellowLedState = LOW;
    }
    digitalWrite(TL_AY, yellowLedState);
    digitalWrite(TL_BY, yellowLedState);
    previousMillisYellowBlink = millis();
  }
}

void changeLight(int turnOff, int turnOn)
{
  digitalWrite(turnOff, LOW);
  digitalWrite(turnOff, HIGH);
}

void receiveRED()
{
  blinkYellow = false;
  changeLight(TL_AG, TL_AY);
  delayMilliseconds();
  changeLight(TL_AY, TL_AR);
  changeLight(PL_AR, PL_AG);
  timerActivated = false;
  changeLight(TL_BR, TL_BY);
  delayMilliseconds();
  changeLight(TL_BY, TL_BG);
}

void receiveGREEN()
{
  blinkYellow = false;
  changeLight(TL_BG, TL_BY);
  delayMillisecondsPedestrian();
  changeLight(TL_BY, TL_BR);
  changeLight(PL_AG, PL_AR);
  changeLight(TL_AR, TL_AY);
  delayMillisecondsPedestrian();
  changeLight(TL_AY, TL_AG);
}

void delayMilliseconds()
{
  unsigned long startTime = millis();
  while (millis() - startTime < yellowInterval)
  {
    // Do nothing
  }
}

void delayMillisecondsPedestrian()
{
  unsigned long startTime = millis();
  while (millis() - startTime < yellowInterval)
  {
    checkPedestrianButton();
  }
}

void receiveOFF()
{
  digitalWrite(PL_AG, LOW);
  digitalWrite(PL_AR, LOW);
  digitalWrite(TL_AG, LOW);
  digitalWrite(TL_AR, LOW);
  digitalWrite(TL_BG, LOW);
  digitalWrite(TL_BR, LOW);
  blinkYellow = true;
}

void receiveMessage()
{
  Message<char[4]> msg = studentNode.receiveSimpleMessage<char[4]>();
  switch (msg.type)
  {
  case RED:
    receiveRED();
    setACK();
    studentNode.sendMessage(controller_name, ACK, ack);
    break;
  case GREEN:
    receiveGREEN();
    setACK();
    studentNode.sendMessage(controller_name, ACK, ack);
    break;
  case OFF:
    receiveOFF();
    setACK();
    studentNode.sendMessage(controller_name, ACK, ack);
    break;
  case PING:
    setSTATUS();
    studentNode.sendMessage(controller_name, ACK, ack);
    break;
  }
}

void setACK()
{
  uint16_t this_id = studentNode.getNodeID(name);
  uint16_t controller_id = studentNode.getNodeID(controller_name);
  ack[0] = (char)this_id;
  ack[1] = (char)ACK;
  ack[2] = (char)controller_id;
  ack[3] = (char)(this_id + ACK + controller_id);
}

void setSTATUS()
{
  pedestRedFailing = digitalRead(PL_AR);
  redFailing = digitalRead(TL_AR);

  char data[] = {
      pedestRedFailing + '0',
      pedestYellowFailing + '0',
      pedestGreenFailing + '0',
      redFailing + '0',
      yellowFailing + '0',
      greenFailing + '0',
      timerActivated + '0',
      '0'};

  int information = strtol(data, NULL, 2);
  uint16_t this_id = studentNode.getNodeID(name);
  uint16_t controller_id = studentNode.getNodeID(controller_name);
  status[0] = (char)this_id;
  status[1] = (char)STATUS;
  status[2] = (char)controller_id;
  status[3] = (char)information;
  status[4] = (char)(this_id + information + STATUS + controller_id);
}