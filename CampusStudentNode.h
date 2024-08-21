#ifndef WORK_STUDENT_NODE_H
#define WORK_STUDENT_NODE_H

#include "StudentNode.h"

class CampusStudentNode : public StudentNode
{
public:
    CampusStudentNode(uint16_t sensorNode, char *name, int channel);

    void init() override;
    void receivePayload() override;

    void performEssentialOperations();
    void sendReadingsRequestToSensorNode();
    Sensor_Node receiveReadingsFromSensorNode();
    void sendAlertRequestToSensorNode(char type, int value);
    void sendAlertDeactivationToSensorNode();
    Alert_Request receiveAlertFromSensorNode();
    uint16_t getNodeID(char *name_pointer);

    /// @brief Override of the sendPayload method to only allow certain message types.
    /// @param to The node ID to send the message to.
    /// @param type The type of the message.
    /// @param payload The payload to send.
    /// @return True if the message was sent successfully, false otherwise.
    template <typename T>
    bool CampusStudentNode::sendPayload(uint16_t to, char type, const T &payload)
    {

        if (type == SELF_ID_REQUEST || type == ID_REQUEST || type == ALERT_REQUEST || type == READINGS_REQUEST)
        {
            log(F(": Message types 'A', 'I', 'N', and 'R' are reserved"));
            return false;
        }

        bool ok = Node::sendPayload(to, type, payload);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
        return ok;
    }

    /// @brief Sends a radio message to a specific node.
    /// @details This function first sends an ID request to the sensor node to get the destination ID.
    /// It then waits to receive the node ID and sends the radio message to that node.
    /// @param name_pointer The name of the destination node.
    /// @param type The type of the message.
    /// @param message The message to send.
    template <typename T>
    void sendMessage(char *name_pointer, char type, const T &message)
    {
        if (type == SELF_ID_REQUEST || type == ID_REQUEST || type == ALERT_REQUEST || type == READINGS_REQUEST)
        {
            log(F(": Message types 'A', 'I', 'N', and 'R' are reserved"));
            return;
        }

        char name[NAME_LENGTH];
        strcpy(name, name_pointer);
        // log(F(": ID request sent to "), _sensorNode, F("(with name "), name, F(")"));
        bool ok = Node::sendPayload(_sensorNode, ID_REQUEST, name);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
        receivePayload();

        log(F(": Message sent to "), nodeID);
        ok = Node::sendPayload(nodeID, type, message);
        countFailedMessages = ok ? 0 : countFailedMessages + 1;
    }

private:
    uint16_t _sensorNode;

    unsigned long last_sent_keep_alive; // When did we send the last keep alive?
    int countFailedMessages = 0;
    uint16_t nodeID = 0;

    void sendKeepAlive(const unsigned long interval);
    Sensor_Node deserializeSensorNode(uint8_t* buffer);
    void serializeAlert(const Alert_Request &temp, uint8_t *buffer);
    Alert_Request deserializeAlert(uint8_t* buffer);
    void restart();
};

#endif