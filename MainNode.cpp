#include "MainNode.h"
#include "MainNode_config.h"

/// @brief Constructor for the MainNode class.
/// @param node  The node ID of the main node.
/// @param channel  The RF24 channel to use.
/// @param ssid  The WiFi SSID to connect to.
/// @param wifiPassword  The WiFi password to use.
/// @param server  The MQTT server to connect to.
/// @param port  The MQTT server port to connect to.
/// @param topic  The MQTT topic to publish to.
MainNode::MainNode(uint16_t node, int channel)
    : PreInstalledNode(channel, node), client(espClient), _ssid(SSID), _wifiPassword(WIFI_PASSWORD), _server(SERVER), _port(PORT), _topic(TOPIC) {}

/// @brief Initializes the main node.
/// @details This function initializes the main node by setting up the WiFi connection, the RF24 network, and the MQTT connection.
void MainNode::init()
{
  delay(INIT_DELAY); // delay 2-5s to prevent from running the code twice
  setupWiFi();
  client.setServer(_server, _port);
  log(F(": Node ID set to "), _node);
  setupRF24Network();
}

/// @brief Sets up the WiFi connection.
void MainNode::setupWiFi()
{
  log(F(": Attempting WiFi connection..."));
  WiFi.begin(_ssid, _wifiPassword);     // Attempt to connect
  while (WiFi.status() != WL_CONNECTED) // Loop until we're reconnected
  {
    log(F(": WiFi connection failed, rc="), WiFi.status(), F(" try again in 5 seconds"));
    delay(WIFI_CONNECT_DELAY); // Wait 5 seconds before retrying
  }
}

/// @brief Checks the MQTT connection.
void MainNode::checkMQTTConnection()
{
  if (!client.connected())
  {
    setupMQTT();
  }
  client.loop();
}

/// @brief Sets up the MQTT connection.
void MainNode::setupMQTT()
{
  while (!client.connected()) // Loop until we're reconnected
  {
    log(F(": Attempting MQTT connection..."));
    if (!client.connect("arduinoClient")) // Attempt to connect
    {
      log(F(": MQTT connection failed, rc="), client.state(), F(" try again in 5 seconds"));
      delay(MQTT_CONNECT_DELAY); // Wait 5 seconds before retrying
    }
  }
}

/// @brief Receives a payload from a specific node.
/// @details This function updates the network and checks if there is any payload available.
/// If there is, it reads the header and processes the payload.
void MainNode::receivePayload()
{
  network.update(); // Pump the network regularly
  while (network.available())
  { // Is there anything ready for us?

    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if (header.type == KEEP_ALIVE)
    {
      receiveKeepAlive(header);
    }
    if (header.type == READINGS_REQUEST)
    {
      receiveReadings(header);
    }
    if (header.type == BEGIN_FLAG)
    {
      receiveBeginFlag(header);
    }
    if (header.type == ACTIVE_NODES)
    {
      receiveNodeID(header);
    }
    if (header.type != KEEP_ALIVE && header.type != READINGS_REQUEST && header.type != BEGIN_FLAG && header.type != ACTIVE_NODES)
    {
      log(F("*** WARNING *** Unknown message type "), header.type);
      network.read(header, 0, 0);
    }
  }
}

/// @brief Receives a keep alive message from a specific node.
/// @details This function updates the network status and its timestamp.
/// @param header  The RF24Network header.
void MainNode::receiveKeepAlive(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);

  network_status[header.from_node - 1].status = true;
  network_status[header.from_node - 1].time = millis();

  log(F(": Keep alive received from "), header.from_node);
}

/// @brief Receives readings from a specific node.
/// @details This function updates the sensor node temperature, phototransistor and name with the information received.
/// @param header  The RF24Network header.
void MainNode::receiveReadings(RF24NetworkHeader &header)
{
  Sensor_Node temp;
  network.read(header, &temp, sizeof(temp));

  network_status[header.from_node - 1].data.temperature = temp.temperature;
  network_status[header.from_node - 1].data.phototransistor = temp.phototransistor;
  strcpy(network_status[header.from_node - 1].data.name, temp.name);

  log(F(": Readings received from "), temp.name, F(" - [temp: "), temp.temperature, F("; light: "), temp.phototransistor, F("]"));
}

/// @brief Receives a begin flag from a specific node.
/// @details This function resets the connected nodes list of the node that sent the begin flag.
/// @param header  The RF24Network header.
void MainNode::receiveBeginFlag(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);

  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    network_status[header.from_node - 1].connected_nodes[i].nodeID = 0;
  }

  log(F(": Begin flag received from "), header.from_node);
}

/// @brief Receives a node ID from a specific node.
/// @details This function updates the connected nodes list of the node that sent the node ID.
/// @param header  The RF24Network header.
void MainNode::receiveNodeID(RF24NetworkHeader &header)
{
  Student_Node message;
  network.read(header, &message, sizeof(message));

  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    if (network_status[header.from_node - 1].connected_nodes[i].nodeID == 0)
    {
      network_status[header.from_node - 1].connected_nodes[i].nodeID = message.nodeID;
      strcpy(network_status[header.from_node - 1].connected_nodes[i].name, message.name);
      break;
    }
  }

  log(F(": Node ID received from "), header.from_node, F(" ("), message.name, F(")"));
}

/// @brief Checks the connection of the sensor nodes.
/// @details This function inactivates the sensor nodes that are not active.
void MainNode::checkNodesConnection()
{
  for (int i = 0; i < MAX_SENSOR_NODES; i++)
  {
    if (network_status[i].status && millis() - network_status[i].time > NODE_CONNECTION_CHECK_INTERVAL)
    {
      network_status[i].status = false;
      log(F(": Node "), i + 1, F(" removed from active nodes list"));
    }
  }
}

/// @brief Publishes the network status to the MQTT server.
/// @details This function sends the temperature, phototransistor, name, and connected nodes of each sensor node.
void MainNode::publishNetworkStatus()
{
  unsigned long now = millis();
  if (now - last_sent > NETWORK_STATUS_SEND_INTERVAL)
  {
    last_sent = now;

    for (int i = 0; i < MAX_SENSOR_NODES; i++)
    {
      if (network_status[i].status)
      {
        DynamicJsonDocument jsonDoc(512);
        JsonObject root = jsonDoc.to<JsonObject>();
        root["Temp"] = network_status[i].data.temperature;
        root["Light"] = network_status[i].data.phototransistor;

        JsonArray nodes = root.createNestedArray("Nodes");

        JsonObject node = nodes.createNestedObject();
        node["id"] = "0" + String(i + 1);
        node["name"] = network_status[i].data.name;

        for (int j = 0; j < MAX_STUDENT_NODES; j++)
        {
          if (network_status[i].connected_nodes[j].nodeID != 0)
          {
            JsonObject node = nodes.createNestedObject();
            node["id"] = "0" + String(decimalToOctal(network_status[i].connected_nodes[j].nodeID));
            node["name"] = network_status[i].connected_nodes[j].name;
          }
        }

        String jsonString;
        serializeJson(root, jsonString);

        log(F(": Sent network status: "), jsonString);
        client.publish(_topic, jsonString.c_str());
      }
    }
  }
}

/// @brief  Converts a decimal number to an octal number.
/// @param decimalNumber The decimal number to convert.
/// @return The octal number.
int MainNode::decimalToOctal(uint16_t decimalNumber)
{
  int octalNumber = 0;
  int base = 1;
  while (decimalNumber != 0)
  {
    octalNumber += (decimalNumber % 8) * base;
    decimalNumber /= 8;
    base *= 10;
  }
  return octalNumber;
}