#include "CampusStudentNode.h"

CampusStudentNode::CampusStudentNode(uint16_t sensorNode, char *name, int channel)
    : StudentNode(NODE_BASE + sensorNode, name, channel), _sensorNode(sensorNode) {}

/// @brief Initializes the student node.
/// @details This function initializes the student node by setting up the RF24Network.
/// It also sends an ID request to the sensor node to get a new node ID.
void CampusStudentNode::init()
{
    delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice

    log(F(": Initial node ID set to "), _node);
    setupRF24Network();
    uint16_t temp = _node;
    while (_node == temp) // wait until the node ID changes
    {
        log(F(": New ID request sent to "), _sensorNode);
        Node::sendPayload(_sensorNode, SELF_ID_REQUEST, _name);
        receivePayload();
        delay(ID_REQUEST_DELAY); // retry after 5s
    }
}

/// @brief Performs the essential operations for the student node.
/// @details This function sends a keep alive message to the sensor node and restarts the connection if the message fails.
void CampusStudentNode::performEssentialOperations()
{
    sendKeepAlive(KEEP_ALIVE_INTERVAL);
    restart();
}

/// @brief Receives a payload from the sensor node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void CampusStudentNode::receivePayload()
{
    network.update(); // Pump the network regularly
    while (network.available())
    {                             // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        network.peek(header);

        if (header.type == SELF_ID_REQUEST)
        {
            network.read(header, &_node, sizeof(_node));
            setupRF24Network();
            log(F(": New node ID received "), _node, F(" from "), header.from_node);
        }
        if (header.type == ID_REQUEST)
        {
            network.read(header, &nodeID, sizeof(nodeID));
            log(F(": Node ID received "), nodeID, F(" from "), header.from_node);
        }
        else
            break;
    }
}

/// @brief Sends a readings request to the sensor node.
void CampusStudentNode::sendReadingsRequestToSensorNode()
{
    log(F(": Readings request sent to "), _sensorNode);
    Node::sendPayload(_sensorNode, READINGS_REQUEST, 0);
}

/// TODO: juntar à de cima para evitar erros dos alunos
/// @brief Receives the readings from the sensor node.
/// @return The readings received from the sensor node.
Sensor_Node CampusStudentNode::receiveReadingsFromSensorNode()
{
    network.update(); // Pump the network regularly
    while (network.available())
    {                             // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        network.peek(header);

        if (header.type == READINGS_REQUEST)
        {
            Sensor_Node temp;
            network.read(header, &temp, sizeof(temp));
            log(F(": Sensor readings received from "), header.from_node, F(" - [temp: "), temp.temperature, F("; light: "), temp.phototransistor, F("]"));
            return temp;
        }
        else
            break;
    }
}

/// @brief Sends an alert request to the sensor node.
/// @param type The type of the alert. It can be 'T' for temperature or 'L' for light.
/// @param value The value of the alert.
void CampusStudentNode::sendAlertRequestToSensorNode(char type, int value)
{
    Alert_Request message;
    message.type = type;
    message.value = value;

    log(F(": Alert activation sent to "), _sensorNode, F(" - [type: "), type, F("; value: "), value, F("]"));
    Node::sendPayload(_sensorNode, ALERT_REQUEST, message);
}

/// @brief Sends an alert deactivation to the sensor node.
void CampusStudentNode::sendAlertDeactivationToSensorNode()
{
    log(F(": Alert deactivation sent to "), _sensorNode);
    Node::sendPayload(_sensorNode, ALERT_DEACTIVATION, 0);
}

/// @brief Receives an alert from the sensor node.
/// @return The alert received from the sensor node.
Alert_Request CampusStudentNode::receiveAlertFromSensorNode()
{
    network.update(); // Pump the network regularly
    while (network.available())
    {                             // Is there anything ready for us?
        RF24NetworkHeader header; // If so, take a look at it
        network.peek(header);

        if (header.type == ALERT_REQUEST)
        {
            Alert_Request temp;
            network.read(header, &temp, sizeof(temp));
            log(F(": Sensor alert received from "), header.from_node, F(" - [type: "), temp.type, F("; value: "), temp.value, F("]"));
            return temp;
        }
        else
            break;
    }
}

/// @brief Gets the node ID of a node with a specific name.
/// @param name_pointer The name of the node.
/// @return The node ID of the node with the specific name.
uint16_t CampusStudentNode::getNodeID(char *name_pointer)
{
    char name[NAME_LENGTH];
    strcpy(name, name_pointer);
    // log(F(": ID request sent to "), _sensorNode, F("(with name "), name, F(")"));
    Node::sendPayload(_sensorNode, ID_REQUEST, name);
    receivePayload();

    if (nodeID == 0) // If the node ID is not found, return
    {
        log(F(": Node ID not found"));
        return 0;
    }

    return nodeID;
}

/// @brief Sends a keep alive message to the sensor node at regular intervals.
/// @param interval The interval at which to send the keep alive message.
void CampusStudentNode::sendKeepAlive(const unsigned long interval)
{
    unsigned long now = millis();
    if (now - last_sent_keep_alive >= interval)
    { // If it's time to send a message, send it!
        last_sent_keep_alive = now;
        log(F(": Keep alive sent to "), _sensorNode);
        Node::sendPayload(_sensorNode, KEEP_ALIVE, 0);
    }
}

/// @brief Restarts the node connection if the number of failed messages exceeds the maximum limit.
void CampusStudentNode::restart()
{
    if (countFailedMessages >= MAX_FAILED_MESSAGES)
    {
        log(F(": Restarting node connection"));
        _node = NODE_BASE + _sensorNode;
        init();
    }
}
