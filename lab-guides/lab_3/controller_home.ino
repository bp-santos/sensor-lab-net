#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "HomeStudentNode.h"

uint16_t this_node = 00;
char name[NAME_LENGTH] = "BERN01";
int channel = 90;

HomeStudentNode studentNode(this_node, name, channel);

// Message types
const int RED = 0;
const int GREEN = 1;
const int OFF = 2;
const int PING = 3;
const int ACK = 4;
const int STATUS = 5;

// Controller pins
#define CONTROLLER_STATUS 12
#define BUS_ACTIVITY 5
#define CONTROLLER_BUTTON 13
const int potentiometer = A0;

// Junctions
#define JUNCTION_1 01
#define JUNCTION_2 011
#define JUNCTION_3 021
#define JUNCTION_4 031
#define NUMBER_OF_JUNCTIONS 2                                        // 4
int junctions_array[NUMBER_OF_JUNCTIONS] = {JUNCTION_1, JUNCTION_2}; // {JUNCTION_1, JUNCTION_2, JUNCTION_3, JUNCTION_4}

// Controller Button
bool currentControllerButtonState = LOW;
int lastControllerButtonState;

// Control Period
#define MIN_CONTROL_PERIOD 2000
#define MAX_CONTROL_PERIOD 15000
unsigned long previousMillis = 0;
unsigned long remainingTime = 0;
bool pedestrainPressedOnce = false;
int controlPeriodOn = 0;
bool timeIsHalved = 0;

// Other variables
bool controllerON = false;
bool firstTimeInLoop = true;
int firstSequence = true;
int junctionIndexToRED = 0;
int junctionIndexToGREEN = 1;

// Pedestrian button check
int pedestrianButtonCheck = 1000;
unsigned long previousMillisBlink = 0;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // some boards need this because of native USB capability
  }
  studentNode.init();

  pinMode(CONTROLLER_STATUS, OUTPUT);
  pinMode(BUS_ACTIVITY, OUTPUT);
  pinMode(CONTROLLER_BUTTON, INPUT);
}

void loop()
{
  // When turned OFF, the controller must signal all traffic lights to start blinking yellow, going back to the initial state.
  if (!controllerON && firstTimeInLoop)
  {
    firstTimeInLoop = false;
    digitalWrite(CONTROLLER_STATUS, LOW); // (red LED OFF)
    Serial.println("Initial state!");

    for (int i = 0; i < NUMBER_OF_JUNCTIONS; i++)
    {
      char off[] = {(char)this_node, (char)OFF, (char)junctions_array[i], (char)(OFF + junctions_array[i])};
      sendMessage(off); // wait for the acknowledgements
    }
  }

  // It must be possible to turn the controller ON and OFF, pressing the button:
  handleControllerButtonPress();

  // When turned ON
  if (controllerON)
  {
    // , the controller must start a cyclic sequence of control of the roundabout.
    // Starting with entry 1:
    if (firstSequence)
    {
      firstSequence = false;
      // Block entries 2, 3 and 4 - Command entries 2, 3 and 4 to go RED
      for (int i = 1; i < NUMBER_OF_JUNCTIONS; i++)
      {
        char red[] = {(char)this_node, (char)RED, (char)junctions_array[i], (char)junctions_array[i]};
        sendMessage(red); // wait for the acknowledgements
      }
      // command entry 1 to go GREEN
      char green[] = {(char)this_node, (char)GREEN, (char)junctions_array[0], (char)(GREEN + junctions_array[0])};
      sendMessage(green);
    }

    if (!controlPeriodOn)
    {
      previousMillis = millis();
      // Wait a control period [2, 15] seconds (controlled by potentiometer)
      remainingTime = map(analogRead(potentiometer), 0, 1023, MIN_CONTROL_PERIOD, MAX_CONTROL_PERIOD);
      controlPeriodOn = true; // prevent entering this if again
      pedestrainPressedOnce = false;
    }


    if (remainingTime > 0)
    {
      // Send a PING to receive the status of the junctions
      // Will check the pedestrian button state
      if (millis() - previousMillisBlink > pedestrianButtonCheck)
      {
        char ping[] = {(char)this_node, (char)PING, (char)junctions_array[junctionIndexToRED], (char)(PING + junctions_array[junctionIndexToRED])};
        sendMessage(ping);
        previousMillisBlink = millis();
      }

      // If the pedestrian button was pressed, the control period must be halved
      if(timeIsHalved && !pedestrainPressedOnce) {
        remainingTime = remainingTime / 2;
        timeIsHalved = false;
        pedestrainPressedOnce = true;
      }

      remainingTime = remainingTime + previousMillis - millis();
      previousMillis = millis();
    }

    if (remainingTime <= 0)
    {
      controlPeriodOn = false; // reset the control period
      pedestrainPressedOnce = false;
      timeIsHalved = false;

      if (junctionIndexToRED == NUMBER_OF_JUNCTIONS - 1) // reach the limit of junctions to go red
      {
        junctionIndexToGREEN = 0; // first junction goes green
      }

      // Block entry 1 - command entry 1 to go RED
      char red[] = {(char)this_node, (char)RED, (char)junctions_array[junctionIndexToRED], (char)junctions_array[junctionIndexToRED]};
      // Wait for the acknowledgement
      sendMessage(red);

      // Command entry 2 to go GREEN (junctionIndexToGREEN starts from 1)
      char green[] = {(char)this_node, (char)GREEN, (char)junctions_array[junctionIndexToGREEN], (char)(1 + junctions_array[junctionIndexToGREEN])};
      sendMessage(green);

      if (junctionIndexToRED == NUMBER_OF_JUNCTIONS - 1)
      {
        junctionIndexToRED = 0; // next junction to go RED is the first one
      }
      else
      { //, and so on (then 3, 4, 1, ...).
        junctionIndexToRED++;
      }

      junctionIndexToGREEN++;
    }
  }
}

void handleControllerButtonPress()
{
  lastControllerButtonState = currentControllerButtonState;                     // previous button state
  currentControllerButtonState = digitalRead(CONTROLLER_BUTTON);                // current button state
  if (lastControllerButtonState == HIGH && currentControllerButtonState == LOW) // if button pressed
  {
    if (controllerON) // if controller is ON
    {                 // turn OFF controller
      controllerON = false;
      firstTimeInLoop = true;
      firstSequence = true;
      // Initial state of the system must - controller turned OFF
      digitalWrite(CONTROLLER_STATUS, LOW); // (red LED OFF)
    }
    else // if controller is OFF
    {    // turn ON controller
      controllerON = true;
      digitalWrite(CONTROLLER_STATUS, HIGH); // red LED ON
    }
  }
}

// While receiving or sending data the controller's blue LED must blink
void sendMessage(char message[])
{
  int messageType = (int)message[1];
  int destination = (int)message[2];

  startBlinkingBLUE();
  studentNode.sendPayload(destination, 'M', message);
  stopBlinkingBLUE();

  if (messageType == PING)
  {
    startBlinkingBLUE();
    studentNode.sendPayload(destination, 'M', STATUS);
    Message<char[5]> status = studentNode.receiveSimpleMessage<char[5]>();
    stopBlinkingBLUE();
    checkStatus((int)status.content[3]);
  }
  else // if message != PING wait for the acknowledgements
  {
    startBlinkingBLUE();
    studentNode.sendPayload(destination, 'M', ACK);
    Message<char[4]> ack = studentNode.receiveSimpleMessage<char[4]>();
    stopBlinkingBLUE();

    // check the ack
    if ((int)ack.content[0] == destination && (int)ack.content[1] == ACK && (int)ack.content[2] == this_node && (int)ack.content[3] == (destination + ACK))
    {
      Serial.println("ACK is correct!");
    }
    else
    {
      Serial.println("ACK is incorrect!");
      controllerON = false; // Shut down the controller
      firstTimeInLoop = true;
      firstSequence = true;
      digitalWrite(CONTROLLER_STATUS, LOW); // (red LED OFF)
    }
  }
}

void startBlinkingBLUE()
{
  digitalWrite(BUS_ACTIVITY, HIGH);
}

void stopBlinkingBLUE()
{
  digitalWrite(BUS_ACTIVITY, LOW);
}

void checkStatus(int status)
{
  Serial.print("Checking status...");
  int binary[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int i = 7;

  while (status > 0)
  {
    binary[i] = status % 2;
    status = status / 2;
    i--;
  }

  // Simplification: Check only the operation condition of the red LED since it is the most critical to enforce safety
  int pedestRedFailing = binary[0];
  int redFailing = binary[3];

  if (pedestRedFailing || redFailing)
  {
    controllerON = false; // shut down the controller
    firstTimeInLoop = true;
    firstSequence = true;
  }

  if (binary[6] == 1)
  {
    timeIsHalved = true;
  }
}