#include "student_node.h"

/// @brief Constructor for the StudentNode class.
/// @param sensorNode The sensor node ID.
/// @param name The name of the student node.
/// @param channel The radio channel.
StudentNode::StudentNode(uint16_t sensorNode, char *name, int channel)
    : radio(RADIO_CE_PIN, RADIO_CSN_PIN), network(radio), _sensorNode(sensorNode), _node(NODE_BASE + sensorNode), _channel(channel)
{
  strcpy(_name, name);
}

/// @brief Initializes the student node.
/// @details This function initializes the student node by setting up the RF24Network.
/// It also sends an ID request to the sensor node to get a new node ID.
void StudentNode::init()
{
  delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice

  log(F(": Initial node ID set to "), _node);
  setupRF24Network();

  uint16_t temp = _node;
  while (_node == temp) // wait until the node ID changes
  {
    sendIDRequest(SELF_ID_REQUEST, _name);
    receivePayload();
    delay(ID_REQUEST_DELAY); // retry after 5s
  }
}

/// @brief Performs the essential operations for the student node.
/// @details This function sends a keep alive message to the sensor node and restarts the connection if the message fails.
void StudentNode::performEssentialOperations()
{
  sendKeepAlive(KEEP_ALIVE_INTERVAL);
  restart();
  receivePayload();
}

/// @brief Sets up the RF24Network for the student node.
/// @details This function initializes the SPI and the radio hardware.
void StudentNode::setupRF24Network()
{
  SPI.begin();
  if (!radio.begin())
  {
    log(F(": Radio hardware not responding!"));
    while (1)
    {
      // hold in infinite loop
    }
  }
  radio.setPALevel(RF24_PA_LEVEL);
  radio.setChannel(_channel);
  network.begin(_node);
}

/// @brief Sends a payload to a specific node.
/// @param to The node ID to send the payload to.
/// @param type The type of the payload.
/// @param payload The payload to send.
void StudentNode::sendPayload(uint16_t to, char type, const void *payload)
{
  network.update(); // keep the network updated
  RF24NetworkHeader header(to, type);
  delay(PAYLOAD_SEND_DELAY); // ensure reliable connectivity
  bool ok = network.write(header, &payload, sizeof(payload));
  log(ok ? F(" (status = 1)") : F(" (status = 0)"));
  countFailedMessages = ok ? 0 : countFailedMessages + 1;
}

/// @brief Receives a payload from the sensor node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void StudentNode::receivePayload()
{
  network.update(); // Pump the network regularly
  while (network.available())
  {                           // Is there anything ready for us?
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
      log(F(": ID received "), nodeID, F(" from "), header.from_node);
    }
    if (header.type == ALERT_REQUEST)
    {
      Alert_Request temp;
      network.read(header, &temp, sizeof(temp));
      log(F(": Sensor alert received from "), header.from_node, F(" - \"[type: "), temp.type, F("; value: "), temp.value, F("]\""));
    }
    if (header.type == READINGS_REQUEST)
    {
      Sensor_Node temp;
      network.read(header, &temp, sizeof(temp));
      log(F(": Sensor readings received from "), header.from_node, F(" - \"[temp: "), temp.temperature, F("; light: "), temp.phototransistor, F("]\""));
    }
  }
}

/// @brief Receives a message header from a specific node.
/// @return The received message header.
RF24NetworkHeader StudentNode::receiveMessageHeader()
{
  network.update();
  while (network.available())
  {
    RF24NetworkHeader header;
    network.peek(header);
    if (header.type != SELF_ID_REQUEST && header.type != ID_REQUEST && header.type != ALERT_REQUEST && header.type != READINGS_REQUEST)
    {
      return header;
    }
    else
    {
      log(F(": Message types 'A', 'I', 'N', and 'R' are reserved."));
      return;
    }
  }
}

/// @brief Sends an ID request to the sensor node.
/// @param type The type of the ID request.
/// @param name The name to send with the ID request.
void StudentNode::sendIDRequest(char type, const char *name)
{
  log(F(": ID request sent to"), _sensorNode);
  sendPayload(_sensorNode, type, name);
}

/// @brief Sends a keep alive message to the sensor node at regular intervals.
/// @param interval The interval at which to send the keep alive message.
void StudentNode::sendKeepAlive(const unsigned long interval)
{
  unsigned long now = millis();
  if (now - last_sent_keep_alive >= interval)
  { // If it's time to send a message, send it!
    last_sent_keep_alive = now;
    log(F(": Keep alive sent to"), _sensorNode);
    sendPayload(_sensorNode, KEEP_ALIVE, 0);
  }
}

/// @brief Restarts the node connection if the number of failed messages exceeds the maximum limit.
void StudentNode::restart()
{
  if (countFailedMessages >= MAX_FAILED_MESSAGES)
  {
    log(F(": Restarting node connection"));
    _node = NODE_BASE + _sensorNode;
    init();
  }
}

/// @brief Sends a readings request to the sensor node.
void StudentNode::sendReadingsRequestToSensorNode()
{
  log(F(": Readings request sent to"), _sensorNode);
  sendPayload(_sensorNode, READINGS_REQUEST, 0);
}

/// @brief Sends an alert request to the sensor node.
/// @param type The type of the alert. It can be 'T' for temperature or 'L' for light.
/// @param value The value of the alert.
void StudentNode::sendAlertRequestToSensorNode(char type, int value)
{
  Alert_Request message;
  message.type = type;
  message.value = value;

  log(F(": Alert activation sent to"), _sensorNode, F(" - \"[type: "), type, F("; value: "), value, F("]\""));
  sendPayload(_sensorNode, ALERT_REQUEST, &message);
}

/// @brief Sends an alert deactivation to the sensor node.
void StudentNode::sendAlertDeactivationToSensorNode()
{
  log(F(": Alert deactivation sent to"), _sensorNode);
  sendPayload(_sensorNode, ALERT_DEACTIVATION, 0);
}

/// @brief Sends a radio message to a specific node.
/// @details This function first sends an ID request to the sensor node to get the destination ID.
/// It then waits to receive the node ID and sends the radio message to that node.
/// @param name The name of the destination node.
/// @param message The message to send.
void StudentNode::sendMessage(char *name, char type, const void *message)
{
  sendIDRequest(ID_REQUEST, name);
  receivePayload();
  if (nodeID == _sensorNode)
  {
    log(F("You cannot send this message to the sensor node. Use the appropriate function instead."));
    return;
  }
  log(F(": Message sent to"), nodeID, F("( "), name, F(") - \""), (char *)message, F("\""));
  sendPayload(nodeID, type, message);
}

/// @brief Reads a message from the radio network.
/// @param header The header of the message.
/// @param message The message to read.
void StudentNode::readMessage(RF24NetworkHeader header, void *message)
{
  network.read(header, &message, sizeof(message));
}

/// @brief Logs a message to the serial monitor.
/// @tparam ...Args This is a variadic template that accepts any number of arguments.
/// @param ...args This is a parameter pack that accepts any number of arguments.
template <typename... Args>
void StudentNode::log(Args... args)
{
  Serial.print(millis());
  Serial.print(F(": "));
  (Serial.print(args), ...);
  Serial.println();
}