#include "sensor_node.h"

Active_Nodes active_nodes[MAX_STUDENT_NODES];
Sensor_Node sensorData;
RF24 radio(7, 8);
RF24Network network(radio);

unsigned long last_reading;
unsigned long last_status_sent;
unsigned long last_sent_keep_alive;

SensorNode::SensorNode(uint16_t node, char* name, int channel) {
  _node = node;
  strcpy(sensorData.name, name);
  _channel = channel;
}

void SensorNode::init() {
  delay(2000); // delay 2-5s to prevent from running the code twice

  Serial.print(millis());
  Serial.print(F(": Node ID set to "));
  Serial.println(_node);

  populateActiveNodesArray();
  setupRF24Network();
}

void SensorNode::setupRF24Network() {
  SPI.begin();
  if(!radio.begin()){
    Serial.println(F("Radio hardware not responding!"));
    while (1){
      // hold in infinite loop
    }
  }
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(_channel);
  network.begin(_node);
}

void SensorNode::populateActiveNodesArray() {
  int table[MAX_STUDENT_NODES] = {20, 30, 40, 50, 120, 220, 320, 420, 520, 130};/*, 230, 330, 430, 530, 140, 240, 340, 440, 540, 150, 250, 350, 450, 550, 1120, 2120, 3120, 4120, 5120,
                      1220, 2220, 3220, 4220, 5220, 1320, 2320, 3320, 4320, 5320, 1420, 2420, 3420, 4420, 5420, 1520, 2520, 3520, 4520, 5520, 1130, 2130, 3130, 4130, 5130,
                      1230, 2230, 3230, 4230, 5230, 1330, 2330, 3330, 4330, 5330, 1430, 2430, 3430, 4430, 5430, 1530, 2530, 3530, 4530, 5530, 1140, 2140, 3140, 4140, 5140,
                      1240, 2240, 3240, 4240, 5240, 1340, 2340, 3340, 4340, 5340, 1440, 2440, 3440, 4440, 5440, 1540, 2540, 3540, 4540, 5540, 1150, 2150, 3150, 4150, 5150, 
                      1250, 2250, 3250, 4250, 5250, 1350, 2350, 3350, 4350, 5350, 1450, 2450, 3450, 4450, 5450, 1550, 2550, 3550, 4550, 5550};*/

  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    active_nodes[i].node.nodeID = table[i]+1;

    int decimalValue = 0;
    int base = 1; // Base for octal (8^0 = 1 initially)
    
    // Extract digits from right to left and convert to decimal
    int octalNum = active_nodes[i].node.nodeID;
    while (octalNum != 0) {
      int lastDigit = octalNum % 10; // Get the rightmost digit
      decimalValue += lastDigit * base; // Add digit * base to decimal value
      octalNum /= 10; // Move to the next digit
      base *= 8; // Increase base (8^1, 8^2, ...)
    }
    active_nodes[i].node.nodeID = decimalValue;
  }
}

void SensorNode::sendKeepAlive(const unsigned long interval, uint16_t to) {
  network.update();
  unsigned long now = millis();
  if (now - last_sent_keep_alive >= interval) {  // If it's time to send a message, send it!

    RF24NetworkHeader header(to, 'P');

    Serial.print(millis());
    Serial.print(F(": Keep alive sent to "));
    Serial.print(to);
    
    delay(100); // ensure reliable connectivity
    bool ok = network.write(header, 0, 0);
    Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));

    last_sent_keep_alive = now;
  }
}

void SensorNode::receive24RFNetworkMessage() {
  network.update(); // Pump the network regularly

  while (network.available()) { // Is there anything ready for us?
    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if(header.type == 'A') handle_A(header);
    if(header.type == 'D') handle_D(header);
    if(header.type == 'R') {handle_R(header); send_R(header.from_node);}
    if(header.type == 'N') {uint16_t id = handle_N(header);send_N(header.from_node, id);}
    if(header.type == 'P') handle_P(header);
    if(header.type != 'A' && header.type != 'D' && header.type != 'R' && header.type != 'N' && header.type != 'P'){
      Serial.print(F("*** WARNING *** Unknown message type "));
      Serial.println(header.type);
      network.read(header, 0, 0);
    }
  }
}

void SensorNode::handle_A(RF24NetworkHeader& header) {
  Alert_Request temp;
  network.read(header, &temp, sizeof(temp));

  for (int i = 0; i < MAX_STUDENT_NODES; ++i) {
    if (active_nodes[i].alerts[0].type == '\0') {
      active_nodes[i].alerts[0].type = temp.type;
      active_nodes[i].alerts[0].value = temp.value;
      break;
    }
  }

  Serial.print(millis());
  Serial.print(F(": Alert request received from "));
  Serial.println(header.from_node);
}

void SensorNode::handle_D(RF24NetworkHeader& header) {
  network.read(header, 0, 0);

  for (int i = 0; i < MAX_STUDENT_NODES; ++i) {
    if (active_nodes[i].node.nodeID == header.from_node){
      active_nodes[i].alerts[0].type = '\0';
      active_nodes[i].alerts[0].value = 0.0;
      break;
    }
  }

  Serial.print(millis());
  Serial.print(F(": Alert deactivation received from "));
  Serial.println(header.from_node);
}

void SensorNode::handle_R(RF24NetworkHeader& header) {
  network.read(header, 0, 0);

  Serial.print(millis());
  Serial.print(F(": Readings request received from "));
  Serial.println(header.from_node);
}

void SensorNode::send_R(uint16_t to) {
  network.update();
  RF24NetworkHeader header(to, 'R');

  Serial.print(millis());
  Serial.print(F(": Readings sent to "));
  Serial.print(to);

  delay(100); // ensure reliable connectivity
  bool ok = network.write(header, &sensorData, sizeof(sensorData));
  Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
}

uint16_t SensorNode::handle_N(RF24NetworkHeader& header) {
  char message[NAME_LENGTH] = "";
  network.read(header, &message, sizeof(message));

  uint16_t id;
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (!active_nodes[i].status) {
      id = active_nodes[i].node.nodeID;
      active_nodes[i].status = true;
      strcpy(active_nodes[i].node.name, message);
      active_nodes[i].time = millis();
      for (int j = 0; j < MAX_ALERT_PER_STUDENT; ++j) {
        active_nodes[i].alerts[j].type = '\0';
        active_nodes[i].alerts[j].value = 0;
      }
      break;
    }
  }

  Serial.print(millis());
  Serial.print(F(": Node ID request received from "));
  Serial.print(header.from_node);
  Serial.print(F(" with the name \""));
  Serial.print(message);
  Serial.println(F("\""));

  return id;
}

void SensorNode::send_N(uint16_t to, uint16_t id) {
  network.update();
  RF24NetworkHeader header(to, 'N'); 
  
  Serial.print(millis());
  Serial.print(F(": New node ID sent to "));
  Serial.print(to);
  Serial.print(" (id = ");
  Serial.print(id);

  delay(100); // ensure reliable connectivity
  bool ok = network.write(header, &id, sizeof(id));
  Serial.println(ok ? F("; status = 1)") : F("; status = 0)"));

  if(!ok) {
    for (int i = 0; i < MAX_STUDENT_NODES; i++) {
      if (active_nodes[i].status && active_nodes[i].node.nodeID == id) {
        active_nodes[i].status = false;
        break;
      }
    }
  }
}

void SensorNode::handle_P(RF24NetworkHeader& header) {
  network.read(header, 0, 0);

  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (active_nodes[i].node.nodeID == header.from_node) {
      active_nodes[i].status = true;
      active_nodes[i].time = millis();
      break; 
    }
  }

  Serial.print(millis());
  Serial.print(F(": Keep alive received from "));
  Serial.println(header.from_node);
}

void SensorNode::send_B(uint16_t to) {
  network.update();
  RF24NetworkHeader header(to, 'B'); // begin flag

  Serial.print(millis());
  Serial.print(F(": Begin flag sent to "));
  Serial.print(to);

  delay(100); // ensure reliable transmission
  bool ok = network.write(header, 0, 0);
  Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
}

void SensorNode::send_S(uint16_t to) {
  for (int i = 0; i < MAX_STUDENT_NODES; ++i) {
    if (active_nodes[i].status) {
      network.update();
      RF24NetworkHeader header(to, 'S');

      Serial.print(millis());
      Serial.print(F(": Active node ("));
      Serial.print(active_nodes[i].node.name);
      Serial.print(F(") sent to "));
      Serial.print(to);

      delay(100); // ensure reliable transmission
      bool ok = network.write(header, &active_nodes[i].node, sizeof(active_nodes[i].node));
      Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
    }
  }
}

void SensorNode::updateSensorValues(int tempPin, int lightPin, const unsigned long interval) {
  unsigned long now = millis();
  if (now - last_reading > interval) {
    last_reading = now;
    int voltage = analogRead(tempPin) * (5.0 / 1024.0); // convert the reading into voltage
    sensorData.temperature = (voltage - 0.5) * 100;  // convert that voltage into temperature in celsius
    sensorData.phototransistor = analogRead(lightPin);  // get the voltage reading from the phototransistor
  }
}

void SensorNode::fakeSensorValues(const unsigned long interval) {
  unsigned long now = millis();
  if (now - last_reading > interval) {
    last_reading = now;
    sensorData.temperature = random(20, 40);
    sensorData.phototransistor = random(0, 1024);
  }
}

void SensorNode::checkNodesConnection(const unsigned long interval) {
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (active_nodes[i].status && millis() - active_nodes[i].time > interval) {
      active_nodes[i].status = false;

      Serial.print(millis());
      Serial.print(": Node ");
      Serial.print(active_nodes[i].node.nodeID);
      Serial.println(" removed from active nodes list");
    }
  }
}

void SensorNode::sendNetworkStatus(const unsigned long interval, uint16_t to) {
  unsigned long now = millis();
  if (now - last_status_sent > interval) {
    send_R(to);
    send_B(to);
    send_S(to);
    last_status_sent = now;
  }
}

void SensorNode::checkAlerts() {
  for (int i = 0; i < MAX_STUDENT_NODES; ++i) {
    if (active_nodes[i].status){
      if ((active_nodes[i].alerts[0].type == 'T' && sensorData.temperature >= active_nodes[i].alerts[0].value) || 
          (active_nodes[i].alerts[0].type == 'L' && sensorData.phototransistor >= active_nodes[i].alerts[0].value)) {

        Alert_Request temp;
        if (active_nodes[i].alerts[0].type == 'T'){
          temp.value = sensorData.temperature;
          temp.type = 'T';
        }
        else if (active_nodes[i].alerts[0].type == 'L'){
          temp.value = sensorData.phototransistor;
          temp.type = 'L';
        }

        network.update();
        RF24NetworkHeader header(active_nodes[i].node.nodeID, 'A');
        bool ok = network.write(header, &temp, sizeof(temp));
        Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
      }
    }
    else break;
  }
}