# WeatherChimes
## Completed By: Winnie Woo, Will Richards, Carter Peene, Rij Dorfman
---
### [Weather Chimes Wiki](https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/WeatherChimes)

WeatherChimes is an Internet of Things (IoT) project that uses Loom from the OPEnS lab to send weather data from an Arduino Feather M0 to [Max](https://cycling74.com/products/max). Much like how a wind chime converts wind information into sound, WeatherChimes strives to use a variety of weather sensors to gather data and then process that information into media like generative music and visual art for users. 


## Build Guide

[WeatherChimes Build Guide PDF](https://docs.google.com/document/d/1GEz6TniiCkyVJEQ1pW2CY4VUsa4j7f_cYcETQBzS96c/edit?usp=sharing)


## Setting Up Mosquitto
Mosquitto is an MQTT broker used for handling communication with remote devices over the MQTT protocol
* Download the [Mosquitto Broker](https://mosquitto.org/download/)
* Create a mosquitto user/password: the command below will create a user with a name of your choosing, `mosquitto_passwd -c /etc/mosquitto/pwfile *name_of_choice*`.
You will be prompted to enter a password.
* Find the mosquitto.conf file and replace it with the [mosquitto.conf](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/MongoDB/mosquitto.conf) file in this repo. For Windows, it can be located in `C:\Program Files\mosquitto\mosquitto.conf`
* In the mosquitto.conf replace line 34 with the path of the pwfile you just created.
* You are now set up to run the Mosquitto exectubale file to host the MQTT broker
<br>

## Setting Up MongoDB Database
[MongoDB Manual](https://docs.mongodb.com/manual/)\
A MongoDB instance should be running on the same server as the MQTT Broker

When data is recieved by the broker it will parse the topic out into the locations that data is stored in the database.

A basic MongoDB setup should suffice in most instances, remote access may be needed which can be completed [here](https://www.digitalocean.com/community/tutorials/how-to-configure-remote-access-for-mongodb-on-ubuntu-20-04)

It is **recommended** that you utilize [MongoDB Clusters](https://www.mongodb.com/basics/clusters) (Specifically replica sets) for logging data as this will allow you to utilize the Max8 framework with less work.

<br>

## Connecting MQTT to MongodB using Node-RED

* Download [Node-RED](https://nodered.org/#get-started)
* Locate the `.node-red` folder on your machine. On windows it can be located at `C:\Users\*your_username*\.node-red`
* Replace the contents of the `.node-red` folder with all the of files in the [Node-RED](https://github.com/OPEnSLab-OSU/WeatherChimes/tree/main/MongoDB/NodeRed) folder of this repository.
* TO run Node-RED, run the command `node-red` in the terminal
* Navigate to 127.0.0.1:1880. You can do this by running "http://localhost:1880/" on a browser. This allows you to interface with the Node-RED “flows”.

### Modifying the Node-RED flow
* The flow should intially look like this:

![flow](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/Pictures/Node-RED_Flow.png?raw=true)

* The node farthest to the right is the Mongodb node. Click on that node and add your MongoDB cluster that you created earlier into the *Server* field

![server](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/Pictures/Node-RED_server.png?raw=true)

* Next, you need to specify what broker you want to connect to. Open the MQTT node (It looks like “+/+”). Click the pencil icon next to the Server field, this will allow us to add a new MQTT server. On the following page give it a name. Specify the server address and port (Figure 18) and then under security specify the username and password you use to authenticate with the broker.

![broker](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/Pictures/Node-RED_broker.png?raw=true)


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



