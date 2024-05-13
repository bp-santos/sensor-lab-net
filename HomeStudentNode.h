#ifndef HOME_STUDENT_NODE_H
#define HOME_STUDENT_NODE_H

#include "StudentNode.h"

class HomeStudentNode : public StudentNode
{
public:
    HomeStudentNode(uint16_t node, char *name, int channel);

    void init() override;
    void receivePayload() override;
};

#endif