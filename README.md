# WeatherChimes

## Flow Chart of Data
![WeatherChimesFlowChart](https://user-images.githubusercontent.com/44516223/134070618-015b09d8-83c5-4a15-a480-e0deb71371de.jpg)

## Build Guide

## Setting Up HiveMQ Broker
Create a new HiveMQ Broker at this link https://console.hivemq.cloud/  
MQTT basics: https://www.hivemq.com/mqtt-essentials/
Testing connection to the broker: http://www.hivemq.com/demos/websocket-client/

## Setting Up MongodB Database
MongodB Manual: https://docs.mongodb.com/manual/  
**For OPEnS Lab Projects**: Communicate with Chet to gain access to the OPEnS Lab MongodB organization.  
**Other**: Create a new organization in MongodB, https://mongodb.com  

Once you have access to an organization, create a project.  
In the project create cluster.  
After the cluster is created there will be a connect button.  
Follow the steps to create admin access to the database. 
After an admin account is made, chose connect to an application and from the drop down menu choose Node.js.  
Make sure the check box saying "Include full driver code example" is not checked.  
Copy the text from the "@" to the "/". This will be used in later steps referred to as the MongodB unique cluster text.  

## Pass Through Script
The Pass Through Script needs be run on server for the duration of data collection for a project.  
To get the pass through script working properly you will have to install some external packages and change some variables within Pass.js pertaining to your project set-up.  
The first step is to install Node.js. https://nodejs.org/en/download/  
After Node is installed you will need to install using mqtt and mongodb node packages using the node package manager `npm`  
For mqtt: `npm install mqtt --save` https://www.npmjs.com/package/mqtt#install  
For mongodb: `npm install mongodb` https://www.w3schools.com/nodejs/nodejs_mongodb.asp  
Once these packages are installed we can change some variables in the [pass.js file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/pass.js)  

This block of code is near the top of the pass.js file
```
const Mongo_username = "";
const Mongo_password = "";
const Mongo_database = "";
const Mongo_unique_cluster_variable = "";

const HiveMQ_username = "";
const HiveMQ_password = "";
const HiveMQ_broker = "";

```
- Insert your MongodB admin username into the quotes for `Mongo_username`
  - This can be found on the MongodB Project main page in the Database Access tab on the left side of the screen
- Insert your MongodB admin password into the quotes for `Mongo_password`
- Choose a name for the Database you want to send data from devices too, for WeatherChimes use `"Chime"`
- Insert your MongodB MongodB unique cluster text into the quotes for `Mongo_unique_cluster_variable`

- Insert your HiveMQ admin username into the quotes for `HiveMQ_username` 
  - This can be found in https://console.hivemq.cloud/ -> Manage Cluster -> Access Management
- Insert your HiveMQ admin password into the quotes for `HiveMQ_password`
- Insert your HiveMQ broker link into the quotes for `HiveMQ_broker`
   - This can be found in https://console.hivemq.cloud/ -> Manage Cluster -> Overview, Hostname

The Pass through script works by connecting to the HiveMQ broker for you project and the MongodB database you are storing data for your project. When ever a meassage is receved to the HiveMQ broker the message contents is passed into the `database()` function along with the second MQTT topic level. The `database()` function uses parses the message contents to send as the contents of a document in MongodB and uses the passed topic level as the name of the collection the data will be stored under.

For the use in WeatherChimes, the second topic level is the name of the device. The data being sent through the HiveMQ database is in a format that does not need to be altered so it is passed through and parse into a json that MongodB can read and store.


## MQTT Dirty Integration for Loom

WeatherChimes is currently using a dirty integration of the network protocol `MQTT`. Since it is has not been fully integrated into Loom, users need to ensure that the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) is included in their `.ino` project file. It has already been integrated into the [weathercomplete](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/weathercomplete.ino)`.ino` file.  

If you would like to use MQTT for a project that is not using Loom you can build off of examples from the [ArduinoMQTTClient Library](https://github.com/arduino-libraries/ArduinoMqttClient/tree/master/examples) here.

### Here are the steps for integrating the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) into any Loom project. 

1. Put in your WiFi SSID and Pasword into the arduino_Secrets.h file
2. Set-up or gain access to a HiveMQ broker and put username and password into the appropriate line in arduino_secrets.h
3. Make sure that the MQTT.h file is in your project folder.
4. Have the line `#include "MQTT.h"` at the top of your main file `.ino` file
5. Before the Loop and Setup functions in your main `.ino` file put in this MQTT_send function:

```
void MQTT_send(){
//Finds json to be parsed
  JsonObject internal = Feather.internal_json(); 

  JsonObject all_sensor_data = parse_for_send(internal);
   
  int sizeOfJSON = 1024; //JSONencoder.capacity();
  char JSONmessageBuffer[sizeOfJSON];
  serializeJson(all_sensor_data, JSONmessageBuffer,sizeOfJSON); // needs 3rd parameter when using c -string
  serializeJsonPretty(all_sensor_data, Serial);
  
  //Serial.print(all_sensor_data);
  
  char name[20];
  Feather.get_device_name(name); // "name" will be one topic level that the data is published to on HiveMQ
  String groupName = String(name + String(Feather.get_instance_num()));
  String topic = String(String("Chime/") + groupName + String("/data"));

  MQTT_connect(); //connect to the MQTT client
  mqttClient.poll();
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker

  //display data
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print(JSONmessageBuffer);


    
  // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(JSONmessageBuffer); //correctly formatted data
    mqttClient.endMessage();

}
```
6. In your `Loop` function call `MQTT_send()` using the line: `MQTT_send();`


## Max Patch set-up and usage
The main goal of setting up all of the MQTT connectivity and MongodB storage of data was to create a seamless functionality to read data into [Max](https://cycling74.com/products/max). 

The MongodB Connection Max Patch uses a Node.js script to connect to a database and recieve live updated information.  
To have the Node.js script running properly ensure that Node is installed. You can do that [here]( https://nodejs.org/en/download/).  
If this is your first time using the MongodB Max Patch and Node is installed click the the `Setup` button on the patch to install the necessary packages to run the main script for the patch.  

The Setup button is installing the node package manager (npm) and the mongodb node package. 
NPM can also be installed here: https://docs.npmjs.com/downloading-and-installing-node-js-and-npm
mongodb node package can be installed using the line `npm install --mongodb`

