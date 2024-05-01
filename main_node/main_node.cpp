#include "main_node.h"

Network_Status network_status[MAX_SENSOR_NODES];
RF24 radio(7, 8);
RF24Network network(radio);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long last_sent;

MainNode::MainNode(int channel, char *ssid, char *wifiPassword, char *server, short port, char *topic)
{
  _channel = channel;
  _ssid = ssid;
  _wifiPassword = wifiPassword;
  _server = server;
  _port = port;
  _topic = topic;
}

void MainNode::init()
{
  delay(2000); // delay 2-5s to prevent from running the code twice

  setupWiFi();
  client.setServer(_server, _port);

  Serial.print(millis());
  Serial.println(F(": Initial node ID set to 00"));
  setupRF24Network();
}

void MainNode::setupWiFi()
{
  Serial.println("Attempting WiFi connection...");
  WiFi.begin(_ssid, _wifiPassword);       // Attempt to connect
  while (WiFi.status() != WL_CONNECTED) // Loop until we're reconnected
  {
    Serial.print(F("WiFi connection failed, rc="));
    Serial.print(WiFi.status());
    Serial.println(F(" try again in 5 seconds"));

    delay(5000); // Wait 5 seconds before retrying
  }
}

void MainNode::setupMQTT()
{
  while (!client.connected()) // Loop until we're reconnected
  {
    Serial.println("Attempting MQTT connection...");
    if (!client.connect("arduinoClient")) // Attempt to connect
    {
      Serial.print(F("MQTT connection failed, rc="));
      Serial.print(client.state());
      Serial.println(F(" try again in 5 seconds"));

      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void MainNode::setupRF24Network()
{
  SPI.begin();
  if (!radio.begin())
  {
    Serial.println(F("Radio hardware not responding!"));
    while (1)
    {
      // hold in infinite loop
    }
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(_channel);
  network.begin(00);
}

void MainNode::checkMQTTConnection()
{
  if (!client.connected())
  {
    setupMQTT();
  }
  client.loop();
}

void MainNode::receive24RFNetworkMessage()
{
  network.update(); // Pump the network regularly

  while (network.available())
  { // Is there anything ready for us?

    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if (header.type == 'R')
      handle_R(header);
    if (header.type == 'B')
      handle_B(header);
    if (header.type == 'S')
      handle_S(header);
    if (header.type == 'P')
      handle_P(header);
    if (header.type != 'R' && header.type != 'B' && header.type != 'S' && header.type != 'P')
    {
      Serial.print(F("*** WARNING *** Unknown message type "));
      Serial.println(header.type);
      network.read(header, 0, 0);
    }
  }
}

void MainNode::handle_R(RF24NetworkHeader &header)
{
  network.update();
  Sensor_Node temp;
  network.read(header, &temp, sizeof(temp));

  network_status[header.from_node - 1].data.temperature = temp.temperature;
  network_status[header.from_node - 1].data.phototransistor = temp.phototransistor;
  strcpy(network_status[header.from_node - 1].data.name, temp.name);

  Serial.print(millis());
  Serial.print(F(": Readings received from \""));
  Serial.print(temp.name);
  Serial.print(F("\" - \"[temp: "));
  Serial.print(temp.temperature);
  Serial.print(F("; light: "));
  Serial.print(temp.phototransistor);
  Serial.println(F("]\""));
}

void MainNode::handle_B(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);
  for (int i = 0; i < MAX_STUDENT_NODES; i++)
  {
    network_status[header.from_node - 1].connected_nodes[i].nodeID = 0;
  }

  Serial.print(millis());
  Serial.print(F(": Begin flag received from "));
  Serial.println(header.from_node);
}

void MainNode::handle_S(RF24NetworkHeader &header)
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

  Serial.print(millis());
  Serial.print(F(": Node received from "));
  Serial.print(header.from_node);
  Serial.print(F(" ("));
  Serial.print(message.name);
  Serial.println(F(")"));
}

void MainNode::handle_P(RF24NetworkHeader &header)
{
  network.read(header, 0, 0);

  network_status[header.from_node - 1].status = true;
  network_status[header.from_node - 1].time = millis();

  Serial.print(millis());
  Serial.print(F(": Keep alive received from "));
  Serial.println(header.from_node);
}

void MainNode::checkNodesConnection(const unsigned long interval)
{
  for (int i = 0; i < MAX_SENSOR_NODES; i++)
  {
    if (network_status[i].status && millis() - network_status[i].time > interval)
    {
      network_status[i].status = false;

      Serial.print(millis());
      Serial.print(F(": Node "));
      Serial.print(i + 1);
      Serial.println(F(" removed from active nodes list"));
    }
  }
}

void MainNode::publishNetworkStatus(const unsigned long interval)
{
  network_status[0].status = true;
  network_status[0].data.temperature = 25;
  network_status[0].data.phototransistor = 100;
  memcpy(network_status[0].data.name, "NODE01", NAME_LENGTH);
  network_status[0].time = millis();
  network_status[0].connected_nodes[0].nodeID = 21;
  network_status[0].connected_nodes[0].name[0] = 'BERN01';
  network_status[0].connected_nodes[1].nodeID = 31;
  network_status[0].connected_nodes[1].name[0] = 'BERN02';

  unsigned long now = millis();
  if (now - last_sent > interval)
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
            node["id"] = "0" + String(network_status[i].connected_nodes[j].nodeID, DEC);
            node["name"] = network_status[i].connected_nodes[j].name;
          }
        }

        String jsonString;
        serializeJson(root, jsonString);

        Serial.print(millis());
        Serial.print(F(": "));
        Serial.println(jsonString);

        client.publish(_topic, jsonString.c_str());
      }
    }
  }
}