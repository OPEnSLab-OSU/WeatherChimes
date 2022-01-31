# WeatherChimes
## Completed By: Winnie Woo, Will Richards, Carter Peene, Rij Dorfman, Jonah Bidermann
---
### [Weather Chimes Wiki](https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/WeatherChimes)

WeatherChimes is an Internet of Things (IoT) project that uses Loom from the OPEnS lab to send weather data from an Arduino Feather M0 to [Max](https://cycling74.com/products/max). Much like how a wind chime converts wind information into sound, WeatherChimes strives to use a variety of weather sensors to gather data and then process that information into media like generative music and visual art for users. 


## Build Guide

[WeatherChimes Build Guide PDF](https://docs.google.com/document/d/1GEz6TniiCkyVJEQ1pW2CY4VUsa4j7f_cYcETQBzS96c/edit?usp=sharing)


## Setting Up Mosquitto
Mosquitto is an MQTT broker used for handling communication with remote devices over the MQTT protocol
* Setup a new [Mosquttio instance](https://www.vultr.com/docs/how-to-install-mosquitto-mqtt-broker-server-on-ubuntu-16-04)
  * To allow for inbound connection we need to listen on all interfaces. This can be done by adding the line `listener 1883 0.0.0.0` to the `mosquitto.conf` file
* [MQTT basics](https://www.hivemq.com/mqtt-essentials/)

<br>

## Setting Up MongoDB Database
[MongoDB Manual](https://docs.mongodb.com/manual/)\
A MongoDB instance should be running on the same server as the MQTT Broker

When data is recieved by the broker it will parse the topic out into the locations that data is stored in the database.

A basic MongoDB setup should suffice in most instances, remote access may be needed which can be completed [here](https://www.digitalocean.com/community/tutorials/how-to-configure-remote-access-for-mongodb-on-ubuntu-20-04)

It is **recommended** that you utilize [MongoDB Clusters](https://www.mongodb.com/basics/clusters) (Specifically replica sets) for logging data as this will allow you to utilize the Max8 framework with less work.

<br>

## Pass Through Script
The Pass Through Script needs to be run on a server for the duration of data collection for a project.  
To get the pass through script working properly, you will have to install some external packages and change some variables within Pass.js pertaining to your project set-up.  
The first step is to install [Node.js](https://nodejs.org/en/download/)
and [node package manager (npm)](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm)

After Node and npm is installed you will need to install MQTT and MongoDB node packages using the node package manager `npm`  
For [MQTT](https://www.npmjs.com/package/mqtt#install): `npm install mqtt --save`  
For [MongoDB](https://www.w3schools.com/nodejs/nodejs_mongodb.asp): `npm install mongodb` 

Once these packages are installed we can change some variables in the [MQTT2Mongo.js file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/pass.js)  

This block of code is near the top of the MQTT2Mongo.js file
```
const Mongo_username = "";
const Mongo_password = "";

const MQTT_username = "";
const MQTT_password = "";
const MQTT_broker = "";

```
- Insert your MongoDB admin username into the quotes for `Mongo_username`
  - This can be found on the MongoDB Project main page in the Database Access tab on the left side of the screen

- Insert your MongoDB admin password into the quotes for `Mongo_password`

- Insert your MQTT admin username into the quotes for `MQTT_username` 
- Insert your MQTT admin password into the quotes for `MQTT_password`
- Insert your MQTT broker link into the quotes for `MQTT_broker`

The Pass through script works by subscribing to all topics of format */* on the Mosquitto broker and then connecting to the MongoDB database also running. Whenever a message is recieved by the pass through script the topic is parsed and mapped to specific parts of the Mongo database. The data recieved is then formatted and pushed in.

In the context of WeatherChimes, the first topic level is the database name, and the second topic level is the device name and the instance number which maps to collection within the database on MongoDB. 

<br>

## MQTT Dirty Integration for Loom

WeatherChimes is currently using a dirty integration of the network protocol `MQTT`. Since it has not been fully integrated into Loom, users need to ensure that the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) is included in their `.ino` project file. It has already been integrated into the [weathercomplete](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/weathercomplete.ino)`.ino` file.  

If you would like to use MQTT for a project that is not using Loom, you can build off of examples from the [ArduinoMQTTClient Library](https://github.com/arduino-libraries/ArduinoMqttClient/tree/master/examples) here.

### Here are the steps for integrating the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) into any Loom project. 

1. Put in your WiFi SSID and Password into the `arduino_secrets.h` in the fields `SECRET_SSID` and `SECRET_PASS`, Password can be left blank if the network is open
2. Set-up or gain access to an MQTT broker (In this case Mosquitto). Once again in the `arduino_secrets.h` we need to input the `BROKER_USER` (Usernname to authenticate with the broker), `BROKER_PASSWORD` (Password to authenticate with the broker), `SECRET_BROKER` (Address to listening broker), `BROKER_PORT` (Port the broker is listening on), `SITE_NAME` (Unique Identifier as this is your Database name)
3. Make sure that the MQTT.h file is in your project folder.
4. Have the line `#include "MQTT.h"` at the top of your main file `.ino` file
5. In your `Setup` function call `setup_MQTT()` using the line: `setup_MQTT();`
6. Before the Loop and Setup functions in your main `.ino` file put in this MQTT_send function:

```

void send_MQTT_data(){
  jsonResponse = "";
  doc.clear();

   // Get the internal JSON object of the data
  doc.add(Feather.internal_json(false));
  serializeJson(doc, jsonResponse);

  // Connect to WIFI and the MQTT Broker
  MQTT_connect(ssid, pass, broker, broker_port);

  // Poll the broker to avoid being disconnected by the server
  mqttClient.poll();

  mqttClient.beginMessage(topic);
  mqttClient.print(jsonResponse);
  mqttClient.endMessage();
}
```

7. In your `Loop` function call `send_MQTT_data()` using the line: `send_MQTT_data();`


<br>

## Max Patch set-up and usage
The main goal of setting up all of the MQTT connectivity and MongoDB storage of data was to create a seamless functionality to read data into [Max](https://cycling74.com/products/max). 

### Setting up the Max Patch

The MongoDB Connection Max Patch uses a Node.js script to connect to a database and receive live updated information.  
To have the Node.js script running properly ensure that Node is installed. You can do that [here]( https://nodejs.org/en/download/).  
If this is your first time using the MongoDB Max Patch and Node, click the the `Setup` button on the patch to install the necessary packages to run the main script for the patch.  

The Setup button installs the [node package manager (npm)](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm) and MongoDB node package.   
The MongoDB node package can be installed using the line `npm install --mongodb`  

### Using the Max Patch
After completing the setup the user can now use the Max Patch.

To use the MongoDB Connect Max Patch, the user must input four peices of information:
1. MongoDB admin username
2. MongoDB admin password
3. [MongDB unique cluster variable](https://github.com/OPEnSLab-OSU/WeatherChimes#setting-up-mongodb-database)
4. The name of the database they would like to connect to, it should be in the Pass through script. **Example: WeatherChimes**
5. The name of the device (collection) they would like to connect to. This is declared in the config file of the weathercomplete .ino **Example: Chime1**


After all of this information is typed into the corresponding boxes, click Send, which begins the script and connects to the MongoDB database.

Data will be streamed out of the outlet of the patch, and can be used seamlessly with the Loom Sensor Max Patch to read data.

When connected successfully, if the inlet of the Max Patch recieves a bang it will output the most recent data that was sent to the connected collection. This also happens upon connecting when Send is clicked.

Further additions can be made to this Patch using the MongoDB node package to get aggregated data outputs and much more. 
  - https://docs.mongodb.com/manual/
  - https://docs.mongodb.com/drivers/node/current/



