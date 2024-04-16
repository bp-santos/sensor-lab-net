#include "student_node.h"

RF24 radio(7, 8); //  nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio); // Network uses that radio

unsigned long last_sent_reading;  // When did we send the last readings request?
unsigned long last_sent_alert_deactivation; // When did send the last alert deactivation?
unsigned long last_sent_keep_alive; // When did we send the last keep alive?

int countFailedMessages = 0;

StudentNode::StudentNode(uint16_t sensorNode, int name, int channel) {
  if(sensorNode < 1 || sensorNode > 5) exit(-1);  // The sensor node must be between 1 and 5
  _sensorNode = sensorNode;
  _node = 010 + sensorNode; // The initial node ID is 011, 012, 013, 014 or 015
  _name = name;
  _channel = channel;
}

void StudentNode::init() {
  delay(2000); // delay 2-5s to prevent from running the code twice
  
  Serial.print(millis());
  Serial.print(F(": Initial node ID set to "));
  Serial.println(_node);
  setupRF24Network();

  uint16_t temp = _node;
  while(_node == temp) {
    sendIDRequest();
    receive24RFNetworkResponse();
    delay(5000);
  }
}

void StudentNode::setupRF24Network() {
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

void StudentNode::sendIDRequest(){
  network.update();
  RF24NetworkHeader header(_sensorNode, 'N');

  Serial.print(millis());
  Serial.print(F(": New node ID request to "));
  Serial.print(_sensorNode);

  delay(100); // ensure reliable connectivity
  bool ok = network.write(header, _name, sizeof(_name));
  Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));
}

void StudentNode::receive24RFNetworkResponse(){
  network.update();
  while (network.available()) { // Is there anything ready for us?

    RF24NetworkHeader header; // If so, take a look at it
    network.peek(header);

    // the use of switch case is not recommended
    if(header.type == 'N') handle_N(header); 
    if(header.type == 'R') handle_R(header);
    if(header.type == 'A') handle_A(header);
  }
}

void StudentNode::handle_N(RF24NetworkHeader& header){
  network.read(header, &_node, sizeof(_node));
  setupRF24Network();

  Serial.print(millis());
  Serial.print(F(": New node ID received "));
  Serial.print(_node);
  Serial.print(F(" from "));
  Serial.println(header.from_node);
}

void StudentNode::handle_R(RF24NetworkHeader& header) {
  Sensor_Values temp;
  network.read(header, &temp, sizeof(temp));

  Serial.print(millis());
  Serial.print(F(": Readings received from "));
  Serial.print(header.from_node);
  Serial.print(F(" - \"[temp: "));
  Serial.print(temp.temperature);
  Serial.print(F("; light: "));
  Serial.print(temp.phototransistor);
  Serial.println(F("]\""));
}

void StudentNode::handle_A(RF24NetworkHeader& header) {
  Alert_Request temp;
  network.read(header, &temp, sizeof(temp));

  Serial.print(millis());
  Serial.print(F(": Sensor alert received from "));
  Serial.print(header.from_node);
  Serial.print(F(" - \"[type: "));
  Serial.print(temp.type);
  Serial.print(F("; value: "));
  Serial.print(temp.value);
  Serial.println(F("]\""));
}

void StudentNode::sendReadingsRequest(const unsigned long interval) {
  network.update();
  unsigned long now = millis();
  if (now - last_sent_reading >= interval) {  // If it's time to send a message, send it!
    last_sent_reading = now;
    RF24NetworkHeader header(_sensorNode, 'R');

    Serial.print(millis());
    Serial.print(F(": Readings request sent to "));
    Serial.print(_sensorNode);
    
    delay(100); // ensure reliable connectivity
    bool ok = network.write(header, 0, 0);
    Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));

    if(!ok) countFailedMessages++;
    if(ok) countFailedMessages = 0;
  }
}

void StudentNode::sendKeepAlive(const unsigned long interval) {
  network.update();
  unsigned long now = millis();
  if (now - last_sent_keep_alive >= interval) {  // If it's time to send a message, send it!
    last_sent_keep_alive = now;

    RF24NetworkHeader header(_sensorNode, 'P');

    Serial.print(millis());
    Serial.print(F(": Keep alive sent to "));
    Serial.print(_sensorNode);
    
    delay(100); // ensure reliable connectivity
    bool ok = network.write(header, 0, 0);
    Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));

    if(!ok) countFailedMessages++;
    if(ok) countFailedMessages = 0;
  }
}

void StudentNode::sendAlertRequest(char type, int value) {
  network.update();
  Alert_Request message;
  message.type = type;
  message.value = value;

  RF24NetworkHeader header(_sensorNode, 'A');

  Serial.print(millis());
  Serial.print(F(": Alert activation sent to "));
  Serial.print(_sensorNode);
  Serial.print(F(" - \"[type: "));
  Serial.print(type);
  Serial.print(F("; value: "));
  Serial.print(value);
  Serial.print(F("]\""));

  delay(100); // ensure reliable connectivity
  bool ok = network.write(header, &message, sizeof(message));
  Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));

  if(!ok) countFailedMessages++;
  if(ok) countFailedMessages = 0;
}

void StudentNode::sendAlertDeactivation(const unsigned long interval) {
  network.update();
  unsigned long now = millis();
  if (now - last_sent_alert_deactivation >= interval) {  // If it's time to send a message, send it!
    last_sent_alert_deactivation = now;

    RF24NetworkHeader header(_sensorNode, 'D');

    Serial.print(millis());
    Serial.print(F(": Alert deactivation sent to "));
    Serial.print(_sensorNode);

    delay(100); // ensure reliable connectivity  
    bool ok = network.write(header, 0, 0);
    Serial.println(ok ? F(" (status = 1)") : F(" (status = 0)"));

    if(!ok) countFailedMessages++;
    if(ok) countFailedMessages = 0;
  }
}

void StudentNode::restart() {
  if (countFailedMessages >= 5) {
    Serial.println(F("Restarting node connection"));
    _node = 010 + _sensorNode;
    init();
  }
}