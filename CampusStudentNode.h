#ifndef WORK_STUDENT_NODE_H
#define WORK_STUDENT_NODE_H

#include "StudentNode.h"

class CampusStudentNode : public StudentNode
{
public:
    CampusStudentNode(uint16_t sensorNode, char *name, int channel);
    
    void init() override;
    RF24NetworkHeader receivePayload() override;

    void performEssentialOperations();
    void sendReadingsRequestToSensorNode();
    void sendAlertRequestToSensorNode(char type, int value);
    void sendAlertDeactivationToSensorNode();
    void sendMessage(char *name, char type, const void *message);

private:
    uint16_t _sensorNode;

    unsigned long last_sent_keep_alive; // When did we send the last keep alive?
    int countFailedMessages = 0;
    uint16_t nodeID;

    void sendKeepAlive(const unsigned long interval);
    void restart();
};

#endif