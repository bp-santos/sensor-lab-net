## Future Work

- Provide descriptions for each node to indicate its geographical location.
- Send student data to the main node for specific topic dissemination.
- Implement automatic adjustment of transmission speed based on error rates.

## Logs

**01/02/2024**

- Initial commit with simple library implementation.
- Added `Data_Package` structure.
- Added functions: `readTemperature`, `readPhotoTransistor`, `setupSimpleTransmitter`, `setupSimpleReceiver`, `sendSimpleRadioMessage`, `receiveSimpleRadioMessage`, `printDataPackage`, `setupRF24Network`, `send24RFNetworkMessage`, `receive24RFMessage`.

**12/02/2024**

- Added `SetupRadio()` function.
- Added `PrintWriteStatus()` and `PrintReadStatus()` functions.
- Removed `PrintDataPackage()` function.
- Added functions: `UpdateSensorValues()`, `sendAlerts()`, `registerStudentNode()`, `requestSensorValues()`, `requestAlertActivation()`.

**20/02/2024**

- Merged `setupRadio` function into `setupRF24Network`.
- Removed `printWriteStatus()` and `printReadStatus()` functions.
- Fixed `updateSensorValues` interval issue by moving `previousTime` initialization.
- Added several print statements.
- Resolved printing issues when all data was in a single print statement.
- Fixed `saveAlertParameters` bug.
- Simplified message type checking system.
- Added logic to `checkAlerts()` function.
- Divided alerts by sensor type and requested value for each student node.
- Added logic to `saveAlertParameters()`.
- Removed `registerStudentNode()` logic; each student now has its own alert.

**21/02/2024**

- Added logic to `requestAllAlertsDeactivation()` for student nodes.
- Added logic to `removeAlertParameters()` for sensor nodes.
- Fixed infinite message loop issue by correctly reading messages and checking their type.

**23/02/2024**

- Split `mylibrary` files into three separate libraries: `main_node`, `sensor_node`, and `student_node`.
- Added `receiveSensorNodeNetworkStatus()` function to the main node.
- Added `sendNetworkStatus()` function to the sensor node.

**27/02/2024**

- Removed unnecessary includes from `.cpp` files as they were already in `.h` files.
- Removed `setupSimpleTransmitter` and `setupSimpleReceiver` functions, which are no longer needed.
- Added MQTT logic to the main node: `setupMQTT`, `connectPublisher`, and `publishNetworkStatus` functions.
- Added status logic to the main node to check sensor node connections in the future.

**29/02/2024**

- Started development of the node-red file for the dashboard.

**01/03/2024**

- Modified the node-red file structure with an initial version of the connected nodes tree.

**08/03/2024**

- Removed unnecessary includes from `.cpp` files as they were already in `.h` files.
- Removed `setupSimpleTransmitter` and `setupSimpleReceiver` functions, which are no longer needed.
- Standardized and simplified code.
- Removed `nrf24l01.h` library from includes since it's already included elsewhere.
- Added `getNodeID` logic to student nodes: writes message type 'N' and then receives message type 'N' with the new node ID.
- Fixed tree display issue on the node-red file dashboard.

**19/03/2024**

- Added `active_nodes` logic to the sensor node.
- Added `populateActiveNodesArray()` logic to the sensor node.
- Added an interval to `updateSensorValues()` function on the sensor nodes.
- Added print statements to the sensor nodes.
- Fixed new message handling by changing `else if` to `if`.
- Added logic to send the next free node ID to student node '1' on the sensor node.

**20/03/2024**

- Standardized and simplified code.
- Simplified `network.read` messages to `(header,0,0)` instead of creating unnecessary local variables.
- Simplified `Alert_Request` struct by removing the `sensor_node` argument.
- Removed `Network_Status` structure and added a new field to `Active_Nodes` to include alerts.

**28/03/2024**

- Standardized and simplified code.
- Modified `populateActiveNodesArray()` to allow a new method for assigning node IDs (utilizing a branch of the tree).
- Added numerous `Serial.print(F())` statements.
- When receiving node ID requests, the sensor node now activates the next free node, names it, and adds a timestamp. It also resets the node's alerts to remove prior changes.
- Moved sensor value updates to a single function called `UpdateSensorValues()`.
- Added PING functionality to verify which student nodes are still communicating to prevent zombies.
- Added switch cases to `receive24RFNetworkMessages` function.

**29/03/2024**

- Added a loop to prevent code execution when the radio module does not respond.
- Standardized and simplified code.
- Added numerous `Serial.print(F())` statements.
- Added an interval to `SendNetworkStatus()` function on the sensor nodes.
- Added a loop on the student node to prevent operation while the initial ID is not changed.

**03/04/2024**

- Removed switch cases for message type analysis due to functionality issues in some cases.
- Added 2-second delay during board startup to ensure information is processed only once.
- Added 100 ms delay before writing messages to the radio network to ensure reliable connectivity.
- Changed keep-alive messages between student nodes and sensor nodes to be one-way without requests.
- Fixed initial addressing issue of student nodes (problem with '0' at the beginning).

**04/04/2024**

- Added a new message type between sensor nodes and the main node to signal the beginning of the node array in the tree.
- Added keep-alive messages between sensor nodes and the main node.
- Added function on the main node to check sensor node connections.

**14/04/2024**

- Fixed conversion issue between octal and decimal in the tree.
- Fixed student node connection problem (excessive simultaneous requests).
- Converted tree node IDs to octal format for MQTT transmission.
- Changed `Sensor_Values` back to floats (due to size differences between UNO and Nano ESP32).
- Added `fakeSensorValues()` function to generate random values for temperature and light sensors.

**16/04/2024**

- Fixed bug where the sensor node was automatically activating student nodes when sending their new ID, causing multiple duplicates on the node-red dashboard.
- Added a feature where the sensor node restarts the network connection after failing to send more than 5 messages.
- Main node and sensor nodes now discard unknown message types.
- Simplified the `populateActiveNodesArray()` function.

**17/04/2024**

- Refactored `Sensor_Values` to `Sensor_Node`, which now includes a name for better clarity.
- When sending sensor data, the sensor node also sends its name.
- Student nodes now also send their names to their sensor node for better clarity.
- Refactored `Sensor_Node` to include a new `Node` structure for easier transmission of connected nodes and their names between sensor nodes and the main node.
- Updated transmitted MQTT data to include node names.

**18/04/2024**

- Updated sensor node to work with the Arduino Nano ESP32.
- Changed readings to `int16_t` to ensure compatibility across all boards.
- Node-red tree now displays each node's name alongside its ID.
- Simplified node-red code.

**19/04/2024**

- Updated node-red dashboard tree to include an image for each node.
- Simplified node-red code.

**20/04/2024**

- Simplified node-red code and fixed major bugs.
- Added reference to [https://gist.github.com/mbostock/4339083](https://gist.github.com/mbostock/4339083).

**01/05/2024**

- Adjusted node-red code for local MQTT broker.
- Referenced MQTT code from [pubsubclient example](https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_basic/mqtt_basic.ino).
- Reorganized MQTT logic in the main node.
- Started Lab 1 guide based on ACIC lectures.

**03/05/2024**

- Completed Lab 1 code based on ACIC lectures.
- Simplified `lab-setup.txt`.

**04/05/2024**

- Began setting up Lab 2 guide.
- Simplified student node code.
- Simplified sensor node code.
- Simplified main node code.

**05/05/2024**

- Fixed errors resulting from the previous day's simplifications.
- Restored functionality with the new simplified code.

**09/05/2024**

- Simplified overall code and created new classes to support inheritance.

**13/05/2024**

- Fixed bugs from the previous commit.
- Prepared everything for Lab 2 work.

**14/05/2024**

- Completed Lab 2 for home environment.
- Fixed some bugs from the major update.

**16/05/2024**

- Completed Lab 2 for campus environment.
- Prepared for Lab 3 work.

**22/05/2024**

- Lab 2 fully functional with data from pre-installed sensors.

**28/05/2024**

- Rewrote Lab 1 and 2 guides.
- Updated `MainNode.cpp` to retrieve WiFi and MQTT credentials from `MainNode_config.h`.
  - Modified `MainNode` constructor and `main_node.ino`.
- Simplified `script.py`.
- Created initial version of Lab 3 controller code (home environment).

**29/05/2024**

- Released initial version of Lab 3 access code (home environment).
- Released initial version of Lab 3 access and controller code (campus environment).
- Added a new function to `CampusStudentNode` code to retrieve any node ID by its name.

**22/07/2024**

- Removed 100 ms delay between messages.
- Students can now request multiple sensor alerts from the sensor node.
- Reordered `init()` function for each node type.
- Increased `countFailedMessages` in message sending for CampusStudentNode.
- Adjusted student-node to use a different endianess, fixing discrepancies in readings.
- Fixed alert handling bug in SensorNode.
- Changed alert sending delay by adding a new field to `Alert_Request`.
