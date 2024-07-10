#include "HomeStudentNode.h"

HomeStudentNode::HomeStudentNode(uint16_t node, char *name, int channel)
    : StudentNode(node, name, channel) {}

/// @brief Initializes the student node.
/// @details This function initializes the student node by setting up the RF24Network.
/// It also sends an ID request to the sensor node to get a new node ID.
void HomeStudentNode::init()
{
    delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice

    log(F(": Initial node ID set to "), _node);
    setupRF24Network();
}

/// @brief Receives a payload from a specific node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void HomeStudentNode::receivePayload()
{
    network.update(); // Pump the network regularly
    while (network.available())
    {                             // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        char message[32];
        network.read(header, &message, sizeof(message));
        log(F(": Message received from "), header.from_node, F(": "), message);
    }
}