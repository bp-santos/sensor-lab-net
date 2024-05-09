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
};

#endif