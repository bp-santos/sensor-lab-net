#ifndef PREINSTALLED_NODE_H
#define PREINSTALLED_NODE_H

#include "Node.h"

const int MAX_STUDENT_NODES = 124;
const unsigned long NODE_CONNECTION_CHECK_INTERVAL = 10000;
const unsigned long NETWORK_STATUS_SEND_INTERVAL = 10000;

const char BEGIN_FLAG = 'B';
const char ACTIVE_NODES = 'S';

struct Student_Node
{
  uint16_t nodeID;
  char name[NAME_LENGTH];
};

class PreInstalledNode : public Node
{
public:
  PreInstalledNode(int channel, uint16_t node) : Node(channel, node) {}

private:
  virtual void checkNodesConnection() = 0;
  virtual void receiveKeepAlive(RF24NetworkHeader &header) = 0;
};

#endif