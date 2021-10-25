# WeatherChimes
## Completed By: Rij Dorfman, Winnie Woo, Jonah Bidermann, and Carter Peene

WeatherChimes is an Internet of Things (IoT) project that uses Loom from the OPEnS lab to send weather data from an Arduino Feather M0 to [Max](https://cycling74.com/products/max). Much like how a wind chime converts wind information into sound, WeatherChimes strives to use a variety of weather sensors to gather data and then process that information into media like generative music and visual art for users. 


## Build Guide
For questions regarding the build process, contact [Winnie Woo](mailto:woow@oregonstate.edu?subject=[GitHub]%20Source%20Han%20Sans)

[WeatherChimes Build Guide PDF](https://docs.google.com/document/d/1GEz6TniiCkyVJEQ1pW2CY4VUsa4j7f_cYcETQBzS96c/edit?usp=sharing)


## Setting Up HiveMQ Broker
* Create a new [HiveMQ Broker](https://console.hivemq.cloud/)
* [MQTT basics](https://www.hivemq.com/mqtt-essentials/)
* Testing connection to the [broker](http://www.hivemq.com/demos/websocket-client/)

## Setting Up MongoDB Database
[MongoDB Manual](https://docs.mongodb.com/manual/)\
**For OPEnS Lab Projects**: Contact [Chet Udell](mailto:udellc@oregonstate.edu?subject=[GitHub]%20Source%20Han%20Sans) to gain access to the OPEnS Lab MongoDB organization.  
**Other**: Create a new organization in [MongoDB](https://mongodb.com)

* Once you have access to an organization, create a project, and within the project create a cluster.  
* After the cluster is created click on the "connect" button
* Follow the steps to create admin access to the database. 
* After an admin account is made, choose connect to an application and from the drop down menu choose Node.js  
* Make sure the check box "Include full driver code example" is not checked.  
* Copy the text from the "@" to before the second "." This will be used in later steps referred to as the MongoDB unique cluster variable.  
- Example: ...@**examplecluster.3na0r**.mongodb.net/...

## Pass Through Script
The Pass Through Script needs to be run on a server for the duration of data collection for a project.  
To get the pass through script working properly, you will have to install some external packages and change some variables within Pass.js pertaining to your project set-up.  
The first step is to install [Node.js](https://nodejs.org/en/download/)
and [node package manager (npm)](https://docs.npmjs.com/downloading-and-installing-node-js-and-npm)

After Node and npm is installed you will need to install MQTT and MongoDB node packages using the node package manager `npm`  
For [MQTT](https://www.npmjs.com/package/mqtt#install): `npm install mqtt --save`  
For [MongoDB](https://www.w3schools.com/nodejs/nodejs_mongodb.asp): `npm install mongodb` 
Once these packages are installed we can change some variables in the [pass.js file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/pass.js)  

This block of code is near the top of the pass.js file
```
const Mongo_username = "";
const Mongo_password = "";
const Mongo_unique_cluster_variable = "";

const HiveMQ_username = "";
const HiveMQ_password = "";
const HiveMQ_broker = "";

```
- Insert your MongoDB admin username into the quotes for `Mongo_username`
  - This can be found on the MongoDB Project main page in the Database Access tab on the left side of the screen
- Insert your MongoDB admin password into the quotes for `Mongo_password`
- Insert your MongoDB unique cluster text into the quotes for `Mongo_unique_cluster_variable`

- Insert your HiveMQ admin username into the quotes for `HiveMQ_username` 
  - This can be found in https://console.hivemq.cloud/ -> Manage Cluster -> Access Management
- Insert your HiveMQ admin password into the quotes for `HiveMQ_password`
- Insert your HiveMQ broker link into the quotes for `HiveMQ_broker`
   - This can be found in https://console.hivemq.cloud/ -> Manage Cluster -> Overview, Hostname

The Pass through script works by connecting to the HiveMQ broker and MongoDB database storing data for your project. Whenever a message is received by the HiveMQ broker, the message contents is passed into the `database()` function along with the second MQTT topic level. The `database()` function parses the message contents to send as the contents of a document in MongoDB and uses the passed topic level as the name of the collection the data will be stored under.

In the context of WeatherChimes, the first topic level is the database name, and the second topic level is the device name and also the name of the collection within the database on MongoDB. The data being sent through the HiveMQ database is in a format that does not need to be altered, so it is passed through and parsed as a json that MongoDB can read and store.


## MQTT Dirty Integration for Loom

WeatherChimes is currently using a dirty integration of the network protocol `MQTT`. Since it has not been fully integrated into Loom, users need to ensure that the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) is included in their `.ino` project file. It has already been integrated into the [weathercomplete](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/weathercomplete.ino)`.ino` file.  

If you would like to use MQTT for a project that is not using Loom, you can build off of examples from the [ArduinoMQTTClient Library](https://github.com/arduino-libraries/ArduinoMqttClient/tree/master/examples) here.

### Here are the steps for integrating the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) into any Loom project. 

1. Put in your WiFi SSID and Password at the top of MQTT.h file in variables `ssid` and `pass`
2. Set-up or gain access to a HiveMQ broker and put username and password into the top of MQTT.h
3. Make sure that the MQTT.h file is in your project folder.
4. Have the line `#include "MQTT.h"` at the top of your main file `.ino` file
5. In your `Setup` function call `Wifi_setup()` using the line: `Wifi_setup();`
6. Before the Loop and Setup functions in your main `.ino` file put in this MQTT_send function:

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
7. In your `Loop` function call `MQTT_send()` using the line: `MQTT_send();`


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
4. The name of the database they would like to connect to, it should be in the Pass through script. **Example: Chime**
5. The name of the device (collection) they would like to connect to. This is declared in the config file of the weathercomplete .ino **Example: Chime1**


After all of this information is typed into the corresponding boxes, click Send, which begins the script and connects to the MongoDB database.

Data will be streamed out of the outlet of the patch, and can be used seamlessly with the Loom Sensor Max Patch to read data.

When connected successfully, if the inlet of the Max Patch recieves a bang it will output the most recent data that was sent to the connected collection. This also happens upon connecting when Send is clicked.

Further additions can be made to this Patch using the MongoDB node package to get aggregated data outputs and much more. 
  - https://docs.mongodb.com/manual/
  - https://docs.mongodb.com/drivers/node/current/



