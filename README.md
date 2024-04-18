# Master's Thesis in Wireless Sensor Networks
Master's Thesis in Wireless Sensor Networks (WSN) using Arduino boards, nRF24L01+ PA/LNA modules, the MQTT protocol, and the Node-RED programming tool.

## Advisor comments
- medir a carga util dos 01-05
- problema de saturacao da rede
- pq e q o no 00 nao faz a gestao toda e é dividida entre o 01 e o 05
- medir data rate entre nos na avaliacao de desempenho
- nicla vision arduino, arducam mini rb-adu-20, sen0158 dfrobot

## To-do
- poder gerar mais que um alerta por estudante
- o student-node deve usar um endianess diferente das outras boards por isso os readings ficam todos diferentes}

## Logs

01/02/2024
- Initial commit with simple library implemented
- Added Data_Package structure
- Added readTemperature, readPhotoTransistor, setupSimpleTransmitter, setupSimpleReceiver, sendSimpleRadioMessage, receiveSimpleRadioMessage, printDataPackage, setupRF24Network, send24RFNetworkMessage, receive24RFMessage functions

12/02/2024
- Added SetupRadio() function
- Added PrintWriteStatus() and PrintReadStatus() functions
- Removed PrintDataPackage() function
- Added UpdateSensorValues(), sendAlerts(), registerStudentNode(), requestSensorValues(), requestAlertActivation() functions

20/02/2024
- Merged the setupRadio function inside the setupRF24Network function
- Removed printWriteStatus() and printReadStatus() functions
- Solved updateSensorValues interval problem: the initialization of the previousTime was inside the function
- Added several prints
- Solved printing problem when everything was inside the same print
- Solved saveAlertParameters bug
- Simplified the message type checking system
- Added logic to checkAlerts() function
- Now the alerts are divided by the type of sensor and value requested for each student node
- Added logic to saveAlertParameters()
- Removed registerStudentNode() logic since now each student will have its own alert

21/02/2024
- Added logic to requestAllAlertsDeactivation() to the student node
- Added logic to removeAlertParameters() to the sensor node
- Solved problem of infinite message loop because I was not reding some messages, just checking their type

23/02/2024
- Splitted the mylibrary files into three different libraries: main_node, sensor_node and student_node
- New function called receiveSensorNodeNetworkStatus() on the main node
- New function called sendNetworkStatus() on the sensor node

27/02/2024
- Removed useless includes on the cpp files since they were already on the h files
- Removed setupSimpleTransmitter and setupSimpleReceiver functions since it will not be necessary in the future
- Added MQTT logic to the main node including setupMQTT, connectPublisher and publishNetworkStatus functions
- Added status logic to the main node to check sensor node connections in the future

29/02/2024
- Started developing the node-red file - dashboard

01/03/2024
- Changes to the node-red file structure with a initial version of the connected nodes tree

08/03/2024
- Removed useless includes on the cpp files since they were already on the h files
- Removed setupSimpleTransmitter and setupSimpleReceiver functions since it will not be necessary in the future
- Code standardization and simplification
- Removed nrf24l01.h library from the includes since is already included on the others
- Added getNodeID logic to the student nodes: writes message type 'N' and then receives message type 'N' with the new nodeID
- Solved tree display problem on the node-red file - dashboard

19/03/2024
- Added active_nodes logic to the sensor node
- Added populateActiveNodesArray() logic to the sensor node
- Added interval to updateSensorValues() function on the sensor nodes
- Added some prints to the sensor nodes
- Solved new message received handling by changing the 'else if' to only 'if'
- Added logic to send the next free node ID to the student node '1' on the sensor node

20/03/2024
- Code standardization and simplification
- Simplifyed network.read messages to (header,0,0) instead of creating useless local variables;
- Simplifyed Alert_Request struct by removing the sensor_node argument
- Removed Network_Status structured and added a new campo to the Active_Nodes to include the alerts

28/03/2024
- Code standardization and simplification
- Changed populateActiveNodesArray() code to allow new method of assyning node IDs (inutilizar um branch da tree)
- Added lots of Serial.print(F());
- When receiving nodeID requests the sensor nodes activates the next free node, names it and adds the timestamp. It also resets the alerts of that node to remove prior changes
- The sensor values are now changed inside one function called UpdateSensorValues()
- Added PING functionality to verify which student nodes are still communicating to prevent zombies
- Added switch cases to the recieve24RFNetworkMessages function

29/03/2024
- Loop to prevent code from working when the radio module does not respond
- Code standardization and simplification
- Added lots of Serial.print(F());
- Added interval to SendNetworkStatus() function on the sensor nodes
- Added loop on the student node that prevents from working while the initial ID is not changed

03/4/2024
- Removed switch cases when analyzing the message type: for some reason was not working properly in some cases
- Added 2s delay when starting the boards to ensure that the information would run only once
- Delay of 100 ms before writing messages to the radio network to ensure reliable connectivity
- Changed keep alive messages between the student nodes and the sensor nodes to be just one way without requests
- Solved initial addressing of student nodes ('0' on the begining problem)

04/04/2024
- Added new message type between the sensor nodes and main node to signal the begining of the array of nodes in the tree
- Added keep alive messages between the sensor nodes and the main node
- Added function on the main node to check the sensor node connections

14/04/2024
- Solved tree problem conversion between octal and decimal
- Solved student_node connection problem (it was sending too much requests at the same time)
- Converted tree nodes ID to octal when sending through MQTT
- Changed Sensor_Values to floats again (UNO -> Nano ESP32 differ int sizes)
- Added function called fakeSensorValues() that generates random values for the temperature and light sensors

16/04/2024
- Solved bug where the sensor node was automatically activating the student nodes when sending their new ID which was causing multiple duplicates for some time that were showing on the node red dashboard
- When failing to send more than 5 messages the sensor node restarts the network connection
- Now the main node and the sensor nodes discard unknown message types
- Simplification of the populateActiveNodesArray() function

17/04/2024
- Sensor_Values refactor to Sensor_Node which now includes a name for the node for better understanding.
- When sending sensor data, the sensor node also sends its name for id.
- The student nodes are also sending their name to their sensor node for better understanding.
- Refactoring of Sensor_Node to include the new structure Node important to easily send the connected nodes and their name between the sensor node and the main node.
- Changed the data that is going to be transmitted via MQTT to include the names of every node.

18/04/2024
- Sensor Node changed to work with the Arduino Nano ESP32
- Change readings to int16_t to be supported across all boards
- The node-red tree now displays the name of each node on top of its id
- The code of the node-red was simplified