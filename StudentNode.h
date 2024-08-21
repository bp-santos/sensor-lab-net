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

template <typename T>
struct Message
{
  char type = '\0';
  T content;
};

struct Alert_Request
{
  char type = '\0';
  int16_t value;
  long time;
};

class StudentNode : public Node
{

public:
  /// @brief Receive a simple message
  /// @tparam T The type of the message content
  /// @return The message received
  template <typename T>
  Message<T> receiveSimpleMessage()
  {
    network.update(); // Pump the network regularly
    Message<T> message;
    while (network.available())
    {                           // Is there anything ready for us?
      RF24NetworkHeader header; // If so, take a look at it
      network.read(header, &message.content, sizeof(message.content));
      message.type = header.type;
      log(F(": Message received from "), header.from_node, F(" type "), (char)header.type);
    }
    return message;
  }

protected:
  char _name[NAME_LENGTH];

  StudentNode(uint16_t node, char *name, int channel)
      : Node(channel, node)
  {
    strcpy(_name, name);
  }
};

#endif