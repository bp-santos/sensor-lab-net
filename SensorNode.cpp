#include "SensorNode.h"

/// @brief Constructor for the SensorNode class.
/// @param node The sensor node ID.
/// @param name The name of the sensor node.
/// @param channel The radio channel.
SensorNode::SensorNode(uint16_t node, char *name, int channel, uint16_t mainNode)
    : PreInstalledNode(channel, node), _mainNode(mainNode)
{
  strcpy(sensorData.name, name);
}

/// @brief Initializes the sensor node.
/// @details This function initializes the sensor node by setting up the RF24Network.
/// It also populates the active nodes array with the node IDs.
void SensorNode::init()
{
  delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice
  log(F(": Node ID set to "), _node);
  setupRF24Network();
  populateActiveNodesArray();
}

/// @brief Populates the active nodes array with the node IDs depending on the sensor node ID.
void SensorNode::populateActiveNodesArray()
{
  int table[MAX_STUDENT_NODES] = {20, 30, 40, 50, 120, 220, 320, 420, 520, 130, 230, 330, 430, 530, 140, 240, 340, 440, 540, 150, 250, 350, 450, 550, 1120, 2120, 3120, 4120, 5120,
                                  1220, 2220, 3220, 4220, 5220, 1320, 2320, 3320, 4320, 5320, 1420, 2420, 3420, 4420, 5420, 1520, 2520, 3520, 4520, 5520, 1130, 2130, 3130, 4130, 5130,
                                  1230, 2230, 3230, 4230, 5230, 1330, 2330, 3330, 4330, 5330, 1430, 2430, 3430, 4430, 5430, 1530, 2530, 3530, 4530, 5530, 1140, 2140, 3140, 4140, 5140,
                                  1240, 2240, 3240, 4240, 5240, 1340, 2340, 3340, 4340, 5340, 1440, 2440, 3440, 4440, 5440, 1540, 2540, 3540, 4540, 5540, 1150, 2150, 3150, 4150, 5150,
                                  1250, 2250, 3250, 4250, 5250, 1350, 2350, 3350, 4350, 5350, 1450, 2450, 3450, 4450, 5450, 1550, 2550, 3550, 4550, 5550};

  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    active_nodes[i].node.nodeID = octalToDecimal(table[i] + _node);
  }
}

/// @brief  Converts an octal number to a decimal number.
/// @param octalNumber The octal number to convert.
/// @return The decimal number.
int SensorNode::octalToDecimal(uint16_t octalNumber)
{
  int decimalValue = 0;
  int base = 1;
  while (octalNumber)
  {
    decimalValue += (octalNumber % 10) * base;
    octalNumber /= 10;
    base *= 8;
  }
  return decimalValue;
}

/// @brief Receives a payload from a specific node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void SensorNode::receivePayload()
{
  network.update(); // Pump the network regularly
  while (network.available())
  {                           // Is there anything ready for us?
    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if (header.type == SELF_ID_REQUEST)
    {
      uint16_t id = receiveNodeIDRequest(header);
      sendNextAvailableNodeID(header.from_node, id);
    }
    if (header.type == ID_REQUEST)
    {
      uint16_t id = receiveNodeIDRequestFromName(header);
      sendNodeID(header.from_node, id);
    }
    if (header.type == ALERT_REQUEST)
    {
      receiveAlertRequest(header);
    }
    if (header.type == ALERT_DEACTIVATION)
    {
      receiveAlertDeactivationRequest(header);
    }
    if (header.type == READINGS_REQUEST)
    {
      receiveReadingsRequest(header);
      sendReadings(header.from_node);
    }
    if (header.type == KEEP_ALIVE)
    {
      receiveKeepAlive(header);
    }
    if (header.type != SELF_ID_REQUEST && header.type != ID_REQUEST && header.type != ALERT_REQUEST && header.type != ALERT_DEACTIVATION && header.type != READINGS_REQUEST && header.type != KEEP_ALIVE)
    {
      log(F("*** WARNING *** Unknown message type "), header.type);
      network.read(header, 0, 0);
    }
  }
}

/// @brief Receives a node ID request from a specific node.
/// @details This function reads the request and stores the node name in the active nodes array.
/// It also activates the next available node, gives it a timestamp and resets the alerts array for the node.
/// @param header The header of the received message.
/// @return The next available node ID.
uint16_t SensorNode::receiveNodeIDRequest(RF24NetworkHeader &header)
{
  char message[NAME_LENGTH] = "";
  network.read(header, &message, sizeof(message));

  uint16_t id;
  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    if (!active_nodes[i].status)
    {
      id = active_nodes[i].node.nodeID;
      active_nodes[i].status = true;
      strcpy(active_nodes[i].node.name, message);
      active_nodes[i].time = millis();
      for (int j = 0; j < MAX_ALERT_PER_STUDENT; ++j)
      {
        active_nodes[i].alerts[j].type = '\0';
        active_nodes[i].alerts[j].value = 0;
      }
      break;
    }
  }

  log(F(": Node ID request received from "), header.from_node, F(" with the name "), message);
  return id;
}

/// @brief  Sends the next available node ID to a specific node.
/// @details This function deactivates the node if the message is not sent successfully.
/// @param to The ID of the node to send the next available node ID to.
/// @param id The next available node ID.
void SensorNode::sendNextAvailableNodeID(uint16_t to, uint16_t id)
{
  log(F(": Next available node ID sent to "), to, F(" (id = "), id, F(")"));
  bool ok = sendPayload(to, SELF_ID_REQUEST, id);

  if (!ok)
  {
    for (int i = 0; i < MAX_STUDENT_NODES; i++)
    {
      if (active_nodes[i].status && active_nodes[i].node.nodeID == id)
      {
        active_nodes[i].status = false;
        break;
      }
    }
  }
}

/// @brief  Receives a node ID request from a specific node.
/// @param header The header of the received message.
/// @return The node ID of the node with the requested name.
uint16_t SensorNode::receiveNodeIDRequestFromName(RF24NetworkHeader &header)
{
  char message[NAME_LENGTH] = "";
  network.read(header, &message, sizeof(message));

  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    if (!strcmp(active_nodes[i].node.name, message) && active_nodes[i].status)
    {
      log(F(": Node ID request received from "), header.from_node, F(" with the name "), message);
      return active_nodes[i].node.nodeID;
    }
  }
  return 00;
}

/// @brief  Sends the node ID to a specific node.
/// @param to The ID of the node to send the node ID to.
/// @param id The node ID to send.
void SensorNode::sendNodeID(uint16_t to, uint16_t id)
{
  log(F(": Node ID sent to "), to, F(" (id = "), id, F(")"));
  sendPayload(to, ID_REQUEST, id);
}

/// @brief  Receives an alert request from a specific node.
/// @details This function reads the alert request and stores it in the active nodes array.
/// @param header The header of the received message.
void SensorNode::receiveAlertRequest(RF24NetworkHeader &header)
{
  uint8_t buffer[3];
  network.read(header, &buffer, sizeof(buffer));
  Alert_Request temp = deserializeAlert(buffer);

  for (int i = 0; i < MAX_STUDENT_NODES; ++i)
  {
    if (active_nodes[i].node.nodeID == header.from_node)
    {
      for (int j = 0; j < MAX_ALERT_PER_STUDENT; ++j)
      {
        if (active_nodes[i].alerts[j].type == '\0')
        {
          active_nodes[i].alerts[j].type = temp.type;
          active_nodes[i].alerts[j].value = temp.value;
          log(F(": Alert request received from "), header.from_node, F(" - [type: "), temp.type, F("; value: "), temp.value, F("]"));
          break;
        }
      }
    }
  }
}

/// @brief Deserializes a buffer into an Alert_Request.
/// @param buffer The buffer that is going to be deserialized
/// @return The Alert_Request it got from the buffer
Alert_Request SensorNode::deserializeAlert(uint8_t* buffer)
{
    Alert_Request temp;
    temp.type = buffer[0];
    temp.value = buffer[1] | (buffer[2] << 8);
    temp.time = buffer[3] | (buffer[4] << 8) | (buffer[5] << 16) | (buffer[6] << 24);
    return temp;
}

/// @brief  Receives an alert deactivation request from a specific node.
/// @details This function deactivates all the alerts for the node.
/// @param header The header of the received message.
void SensorNode::receiveAlertDeactivationRequest(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);
  cleanAlertArray(header.from_node);
  log(F(": Alert deactivation request received from "), header.from_node);
}

/// @brief Receives a node ID and cleans the Alert_Request array from that node
/// @param to The node ID
void SensorNode::cleanAlertArray(uint16_t to)
{
  for (int i = 0; i < MAX_STUDENT_NODES; ++i)
  {
    if (active_nodes[i].node.nodeID == to)
    {
      for (int j = 0; j < MAX_ALERT_PER_STUDENT; ++j)
      {
        active_nodes[i].alerts[j].type = '\0';
        active_nodes[i].alerts[j].value = 0.0;
        active_nodes[i].alerts[j].time = 0;
      }
      break;
    }
  }
}

/// @brief  Receives a readings request from a specific node.
/// @param header The header of the received message.
void SensorNode::receiveReadingsRequest(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);
  log(F(": Readings request received from "), header.from_node);
}

/// @brief  Sends the readings to a specific node.
/// @param to The ID of the node to send the readings to.
void SensorNode::sendReadings(uint16_t to)
{
  log(F(": Readings sent to "), to, F(" - [temp: "), sensorData.temperature, F("; light: "), sensorData.phototransistor, F("]"));
  uint8_t buffer[NAME_LENGTH + 4];
  serializeSensorNode(buffer);
  sendPayload(to, READINGS_REQUEST, buffer);
}

/// @brief Serializes an Sensor_Node into a buffer.
/// @param temp The Sensor_Node that is going to be serialized.
/// @param buffer The buffer it got from the Sensor_Node
void SensorNode::serializeSensorNode(uint8_t *buffer)
{
  memcpy(buffer, sensorData.name, NAME_LENGTH);

  buffer[NAME_LENGTH] = sensorData.temperature & 0xFF;
  buffer[NAME_LENGTH + 1] = (sensorData.temperature >> 8) & 0xFF;

  buffer[NAME_LENGTH + 2] = sensorData.phototransistor & 0xFF;
  buffer[NAME_LENGTH + 3] = (sensorData.phototransistor >> 8) & 0xFF;
}

/// @brief  Receives a keep alive message from a specific node.
/// @details This function updates the status and timestamp of the active nodes.
/// @param header The header of the received message.
void SensorNode::receiveKeepAlive(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);

  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    if (active_nodes[i].node.nodeID == header.from_node)
    {
      active_nodes[i].status = true;
      active_nodes[i].time = millis();
      break;
    }
  }

  log(F(": Keep alive received from "), header.from_node);
}

/// @brief  Sends a keep alive message to the main node at regular intervals.
void SensorNode::sendKeepAlive()
{
  unsigned long now = millis();
  if (now - last_sent_keep_alive >= KEEP_ALIVE_INTERVAL)
  { // If it's time to send a message, send it!
    last_sent_keep_alive = now;
    log(F(": Keep alive sent to "), _mainNode);
    sendPayload(_mainNode, KEEP_ALIVE, 0);
  }
}

/// @brief  Updates the sensor values at regular intervals.
/// @param tempPin The temperature sensor pin.
/// @param lightPin The light sensor pin.
void SensorNode::updateSensorValues(int tempPin, int lightPin)
{
  unsigned long now = millis();
  if (now - last_reading > SENSOR_DATA_UPDATE_INTERVAL)
  {
    last_reading = now;
    int voltage = analogRead(tempPin) * (5.0 / 1024.0); // convert the reading into voltage
    sensorData.temperature = (voltage - 0.5) * 100;     // convert that voltage into temperature in celsius
    sensorData.phototransistor = analogRead(lightPin);  // get the voltage reading from the phototransistor
  }
}

/// @brief  Generates random sensor values at regular intervals.
void SensorNode::generateRandomSensorValues()
{
  unsigned long now = millis();
  if (now - last_reading > SENSOR_DATA_UPDATE_INTERVAL)
  {
    last_reading = now;
    sensorData.temperature = random(20, 40);
    sensorData.phototransistor = random(0, 1024);
  }
}

/// @brief  Checks the connection of the nodes at regular intervals.
/// @details This function checks the connection of the nodes and removes the nodes that are inactive.
void SensorNode::checkNodesConnection()
{
  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    if (active_nodes[i].status && millis() - active_nodes[i].time > NODE_CONNECTION_CHECK_INTERVAL)
    {
      active_nodes[i].status = false;
      cleanAlertArray(active_nodes[i].node.nodeID);
      log(F(": Node "), active_nodes[i].node.nodeID, F(" removed from active nodes list"));
    }
  }
}

/// @brief  Sends the network status to the main node at regular intervals.
void SensorNode::sendNetworkStatus()
{
  unsigned long now = millis();
  if (now - last_status_sent > NETWORK_STATUS_SEND_INTERVAL)
  {
    sendReadings(_mainNode);
    sendBeginFlagArray();
    sendArrayOfActiveNodes();
    last_status_sent = now;
  }
}

/// @brief  Sends a begin flag to the main node.
/// @details The flag indicates the beginning of the data transmission.
void SensorNode::sendBeginFlagArray()
{
  log(F(": Begin flag sent to "), _mainNode);
  sendPayload(_mainNode, 'B', 0);
}

/// @brief  Sends an array of active nodes to the main node.
void SensorNode::sendArrayOfActiveNodes()
{
  for (int i = 0; i < MAX_STUDENT_NODES; ++i)
  {
    if (active_nodes[i].status)
    {
      log(F(": Active node ("), active_nodes[i].node.name, F(") sent to "), _mainNode);
      sendPayload(_mainNode, 'S', active_nodes[i].node);
    }
  }
}

/// @brief  Checks the alerts of the active nodes at regular intervals.
/// @details This function checks the alerts of the active nodes and sends an alert to the student node if the alert condition is met.
void SensorNode::checkAlerts()
{
  for (int i = 0; i < MAX_STUDENT_NODES; ++i)
  {
    if (active_nodes[i].status)
    {
      for (int j = 0; j < MAX_ALERT_PER_STUDENT; ++j)
      {
        if ((active_nodes[i].alerts[j].type == 'T' && sensorData.temperature >= active_nodes[i].alerts[j].value) ||
            (active_nodes[i].alerts[j].type == 'L' && sensorData.phototransistor >= active_nodes[i].alerts[j].value))
        {
          if (millis() - active_nodes[i].alerts[j].time > NODE_ALERT_CHECK_INTERVAL)
          {
            active_nodes[i].alerts[j].time = millis();

            Alert_Request temp;
            if (active_nodes[i].alerts[j].type == 'T')
            {
              temp.value = sensorData.temperature;
              temp.type = 'T';
            }
            else if (active_nodes[i].alerts[j].type == 'L')
            {
              temp.value = sensorData.phototransistor;
              temp.type = 'L';
            }
            log(F(": Alert sent to "), active_nodes[i].node.nodeID, F(" - [type: "), temp.type, F("; value: "), temp.value, F("]"));
            uint8_t buffer[3];
            serializeAlert(temp, buffer);
            sendPayload(active_nodes[i].node.nodeID, ALERT_REQUEST, buffer);
          }
        }
      }
    }
  }
}

/// @brief Serializes an Alert_Request into a buffer.
/// @param temp The Alert_Request that is going to be serialized.
/// @param buffer The buffer it got from the Alert_Request
void SensorNode::serializeAlert(const Alert_Request &temp, uint8_t *buffer)
{
  buffer[0] = temp.type;

  buffer[1] = temp.value & 0xFF;
  buffer[2] = (temp.value >> 8) & 0xFF;

  buffer[3] = temp.time & 0xFF;
  buffer[4] = (temp.time >> 8) & 0xFF;
  buffer[5] = (temp.time >> 16) & 0xFF;
  buffer[6] = (temp.time >> 24) & 0xFF;
}