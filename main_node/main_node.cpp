#include "main_node.h"

Network_Status network_status[MAX_SENSOR_NODES];
RF24 radio(7, 8);
RF24Network network(radio);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long last_sent;

MainNode::MainNode(int channel) {
  _channel = channel;
}

void MainNode::init() {
  delay(2000); // delay 2-5s to prevent from running the code twice
  Serial.print(millis());
  Serial.println(F(": Initial node ID set to 00"));
  setupRF24Network();
}

void MainNode::setupRF24Network() {
  SPI.begin();
  if(!radio.begin()){
    Serial.println(F("Radio hardware not responding!"));
    while (1){
      // hold in infinite loop
    }
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(_channel);
  network.begin(00);
}

void MainNode::setupMQTT(char *ssid, char *wifiPassword, char *server, int port) {
  delay(10);
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  client.setServer(server, port);
}

void MainNode::receive24RFNetworkMessage() {
  network.update(); // Pump the network regularly

  while (network.available()) { // Is there anything ready for us?

    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if(header.type == 'R') handle_R(header);
    if(header.type == 'B') handle_B(header);
    if(header.type == 'S') handle_S(header);
    if(header.type == 'P') handle_P(header);
    if(header.type != 'R' && header.type != 'B' && header.type != 'S' && header.type != 'P'){
      Serial.print(F("*** WARNING *** Unknown message type "));
      Serial.println(header.type);
      network.read(header, 0, 0);
    }
  }
}

void MainNode::handle_R(RF24NetworkHeader& header) {
  network.update();
  Sensor_Values temp;
  network.read(header, &temp, sizeof(temp));

  network_status[header.from_node - 1].data.temperature = temp.temperature;
  network_status[header.from_node - 1].data.phototransistor = temp.phototransistor;

  Serial.print(millis());
  Serial.print(F(": Readings received from "));
  Serial.print(header.from_node);
  Serial.print(F(" - \"[temp: "));
  Serial.print(temp.temperature);
  Serial.print(F("; light: "));
  Serial.print(temp.phototransistor);
  Serial.println(F("]\""));
}

void MainNode::handle_B(RF24NetworkHeader& header) {
  network.read(header, 0, 0);
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    network_status[header.from_node - 1].connected_nodes[i] = 0;
  }

  Serial.print(millis());
  Serial.print(F(": Begin flag received from "));
  Serial.println(header.from_node);
}

void MainNode::handle_S(RF24NetworkHeader& header) {
  uint16_t temp;
  network.read(header, &temp, sizeof(temp));
  
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (network_status[header.from_node - 1].connected_nodes[i] == 0) {
      network_status[header.from_node - 1].connected_nodes[i] = temp;
      break;
    }
  }

  Serial.print(millis());
  Serial.print(F(": Node received from "));
  Serial.print(header.from_node);
  Serial.print(F(" ("));
  Serial.print(temp);
  Serial.println(F(")"));
}

void MainNode::handle_P(RF24NetworkHeader& header) {
  network.read(header, 0, 0);

  network_status[header.from_node - 1].status = true;
  network_status[header.from_node - 1].time = millis();

  Serial.print(millis());
  Serial.print(F(": Keep alive received from "));
  Serial.println(header.from_node);
}

void MainNode::checkNodesConnection(const unsigned long interval) {
  for (int i = 0; i < MAX_SENSOR_NODES; i++) {
    if (network_status[i].status && millis() - network_status[i].time > interval) {
      network_status[i].status = false;

      Serial.print(millis());
      Serial.print(": Node ");
      Serial.print(i+1);
      Serial.println(" removed from active nodes list");
    }
  }
}

void MainNode::connectPublisher(char *username, char *mqttPassword) {
  while (!client.connected()) {
    if (!client.connect("arduinoClient", username, mqttPassword)) {
      delay(5000);
    }
  }
  client.loop();
}

// todo: pensar sobre isto
void MainNode::publishNetworkStatus(char *topic, const unsigned long interval) {
  unsigned long now = millis();
  if (now - last_sent > interval) {
    last_sent = now;

    for (int i = 0; i < MAX_SENSOR_NODES; i++) {
      if (network_status[i].status) {
        String serializedData = "ID: ";
        serializedData += String(i+1);
        serializedData += ", Temp: ";
        serializedData += String(network_status[i].data.temperature);
        serializedData += ", Light: ";
        serializedData += String(network_status[i].data.phototransistor);
        serializedData += ", Nodes: ";

        for (int j = 0; j < MAX_STUDENT_NODES; ++j) {
          if (network_status[i].connected_nodes[j] != 0) {
            char octal[7];
            sprintf(octal, "%o", network_status[i].connected_nodes[j]);
            serializedData += String(octal);
            serializedData += " ";
          }
        }

        Serial.print(millis());
        Serial.print(F(": "));
        Serial.println(serializedData);

        //client.publish(topic, serializedData.c_str());
      }
    }
  }
}