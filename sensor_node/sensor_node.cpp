#include "sensor_node.h"

Active_Nodes active_nodes[MAX_STUDENT_NODES];
Sensor_Values sensorData;
RF24 radio(7, 8);
RF24Network network(radio);

unsigned long last_reading;
unsigned long last_status_sent;
unsigned long last_sent_keep_alive;

SensorNode::SensorNode(uint16_t node, int channel) {
  _node = node;
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
  int currentNumber = _node;
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    int penultimateDigit = (currentNumber / 10) % 10;
    int antepenultimateDigit = (currentNumber / 100) % 10;
    int anteantepenultimateDigit = (currentNumber / 1000) % 10;
    int numDigits = String(currentNumber).length();
    int nextNumber;

    if(numDigits == 1) {
      active_nodes[i].nodeID = 20 + _node;
    }
    if (numDigits == 2 && penultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 100) * 100 + (penultimateDigit + 1) * 10 + (currentNumber % 10);
    }
    if (numDigits == 2 && penultimateDigit == 5) {
      active_nodes[i].nodeID = 120 + _node;
    }
    if (numDigits == 3 && antepenultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 1000) * 1000 + (antepenultimateDigit + 1) * 100 + (currentNumber % 100);
    }
    if (numDigits == 3 && antepenultimateDigit == 5 && penultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 1000) * 1000 + 100 + (penultimateDigit + 1) * 10 + (currentNumber % 10);
    }
    if (numDigits == 3 && antepenultimateDigit == 5 && penultimateDigit == 5) {
      active_nodes[i].nodeID = 1120 + _node;
    }
    if (numDigits == 4 && anteantepenultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 10000) * 10000 + (anteantepenultimateDigit + 1) * 1000 + (currentNumber % 1000);
    }
    if (numDigits == 4 && anteantepenultimateDigit == 5 && antepenultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 10000) * 10000 + 1000 + (antepenultimateDigit + 1) * 100 + (currentNumber % 100);
    }
    if (numDigits == 4 && anteantepenultimateDigit == 5 && antepenultimateDigit == 5 && penultimateDigit < 5) {
      active_nodes[i].nodeID = (currentNumber / 10000) * 10000 + 1000 + 100 + (penultimateDigit + 1) * 10 + (currentNumber % 10);
    }
    currentNumber = active_nodes[i].nodeID;
  }

  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    int decimalValue = 0;
    int base = 1; // Base for octal (8^0 = 1 initially)
    
    // Extract digits from right to left and convert to decimal
    int octalNum = active_nodes[i].nodeID;
    while (octalNum != 0) {
      int lastDigit = octalNum % 10; // Get the rightmost digit
      decimalValue += lastDigit * base; // Add digit * base to decimal value
      octalNum /= 10; // Move to the next digit
      base *= 8; // Increase base (8^1, 8^2, ...)
    }
    active_nodes[i].nodeID = decimalValue;
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
    if (active_nodes[i].nodeID == header.from_node){
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
  int message;
  network.read(header, &message, sizeof(message));

  uint16_t id;
  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (!active_nodes[i].status) {
      id = active_nodes[i].nodeID;
      active_nodes[i].status = true;
      active_nodes[i].name = message;
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
  Serial.println(header.from_node);

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
}

void SensorNode::handle_P(RF24NetworkHeader& header) {
  network.read(header, 0, 0);

  for (int i = 0; i < MAX_STUDENT_NODES; i++) {
    if (active_nodes[i].nodeID == header.from_node) {
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
      Serial.print(active_nodes[i].nodeID);
      Serial.print(F(") sent to "));
      Serial.print(to);

      delay(100); // ensure reliable transmission
      bool ok = network.write(header, &active_nodes[i].nodeID, sizeof(active_nodes[i].nodeID));
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
      Serial.print(active_nodes[i].nodeID);
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
        RF24NetworkHeader header(active_nodes[i].nodeID, 'A');
        bool ok = network.write(header, &temp, sizeof(temp));
        Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
      }
    }
    else break;
  }
}