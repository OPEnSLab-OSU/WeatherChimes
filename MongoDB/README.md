# MongoDB

This folder contains all code for managing the process of retrieving the data from our Mosquitto MQTT Broker and Passing it Along to our MongoDB Database.

---
<br>

## Node Red
This contains flows and charts to get Node Red up and processing MQTT packets

<br>

## Max Patch
This folder contains all required files to get a Max MSP patch up and running for collecting data from the MongoDB database
 - mongodb.maxpat - Patcher for within MaxMSP itself
 - Mongo2Max.js - JavaScript helper function for the patch should be placed within the javascript subfolder inside the Loom folder for Max

<br>

## Mosquitto
The mosquitto.conf file provides a working configuration for running your own MQTT broker on your own linux server. Most parts of this configuration can be copied over to windows granted the file paths are changed.
