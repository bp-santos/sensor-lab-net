#ifndef STUDENT_NODE_H
#define STUDENT_NODE_H

#include "Node.h"

const uint16_t NODE_BASE = 010;
const int MAX_FAILED_MESSAGES = 5;
const unsigned long KEEP_ALIVE_INTERVAL = 3000;
const unsigned long ID_REQUEST_DELAY = 5000;

const char SELF_ID_REQUEST = 'N';
const char ID_REQUEST = 'I';
const char ALERT_REQUEST = 'A';
const char ALERT_DEACTIVATION = 'D';
const char SIMPLE_MESSAGE = 'M';

struct Alert_Request
{
  char type = '\0';
  int value;
};

class StudentNode : public Node
{

public:
  char _name[NAME_LENGTH];

  StudentNode(uint16_t node, char *name, int channel)
      : Node(channel, node)
  {
    strcpy(_name, name);
  }

  /// @brief Receives a payload from a specific node.
  /// @details This function updates the network and checks if there is any payload available.
  /// If there is, it reads the header and processes the payload.
  char *receiveSimpleMessage()
  {
    network.update(); // Pump the network regularly
    char message[32];
    while (network.available())
    {                           // Is there anything ready for us?
      RF24NetworkHeader header; // If so, take a look at it
      network.peek(header);
      if (header.type == SIMPLE_MESSAGE)
      {
        network.read(header, &message, sizeof(message));
        log(F(": Message received from "), header.from_node, F(": "), message);
      }
    }
    return message;
  }
};

#endif