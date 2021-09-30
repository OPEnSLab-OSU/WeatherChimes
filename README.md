# WeatherChimes
## Completed By: Rij Dorfman, Winnie Woo, Jonah Bidermann, and Carter Peene

WeatherChimes is an Internet of Things (IoT) project that uses OPEns Lab API: Loom, to send weather data from an arduino Feather M0 to [Max](https://cycling74.com/products/max). Much Like a wind chime converts wind information into sound, WeatherChimes strives to use a variety of weather sensors to gather data and then process that information into media like light and sound for users.

## Flow Chart of Data
![WeatherChimesFlowChart](https://user-images.githubusercontent.com/44516223/134070618-015b09d8-83c5-4a15-a480-e0deb71371de.jpg)

## Build Guide

The picture below shows a completed WeatherChimes device  

![image](https://user-images.githubusercontent.com/44516223/134250524-097c0fb9-f07d-41c9-a773-49bbf0047ae9.png)

### Material list:

- [WeatherChimes BOM](https://docs.google.com/spreadsheets/d/1uX9fioGPlSb6EPLqSNqS9fljZabWL6rv3EGTAPcsChk/edit?usp=sharing)
- SD Card (16GB)
- Coin cell battery
- SHT30 Weather Proof Temp/Humidity
- TSL2591
- METER GS3
- PG7 Cable Glands (x2)
- Waterproof 4-pin cable set (x2)
- Pelican 1040 clear top case
- FeatherWing Doubler
- Feather M0 WiFi
- Hypnos Board
- WeatherChimes PCB
- 31.00mm threaded hex standoff
- 2.5mm bolts
- Male / Female Headers  (x2 male, x3 females)
- Base plate

### Tool list:

- Soldering iron / solder
- Wire stripper
- Wire cutter
- Exacto knife
- Heat shrink

### Schematic

![image](https://user-images.githubusercontent.com/44516223/134250723-4e4c8b0d-7911-4ad8-9183-0759ebf7c945.png)

### Base Plate 3D print

[WeatherChimes Base Plate Fusion 360 File](https://a360.co/3EDbXnZ)

![image](https://user-images.githubusercontent.com/44516223/134250877-7dd4a7ff-2e05-497b-934d-a38ddf825623.png)

### Procedure:

#### 1. PG7 cable glands

![image](https://user-images.githubusercontent.com/44516223/134251424-bdf81c8e-75a3-47f3-b5a3-16ef2e78c888.png)

The cable glands are shown circled in red. To install the cable glands in the Pelican case, drill a hole with a 7/16" drill bit and use a ½-13 tap to create the threads. Twist the ½-13 NC tapping tool in the hole for one full rotation and then backwards 1/2 a rotation; repeat this until the entire hole is threaded. Then screw in the cable gland.

#### 2. Pelican Case

![image](https://user-images.githubusercontent.com/44516223/134251554-1ddd6538-63a8-49fc-81ef-d4b87d180178.png)

Carefully cut along the red line shown in the image with an exacto blade.

#### 3. FeatherWing Doubler 

![image](https://user-images.githubusercontent.com/44516223/134251672-aaae09f8-e27c-44af-b3e2-1116574eeb99.png)

Solder short female headers on the outer side of the doubler. Note that a Feather board should fit into the headers. 

#### 4. Feather M0 WiFi

![image](https://user-images.githubusercontent.com/44516223/134251724-16824893-f58e-417d-b339-751df45c32c3.png)
![image](https://user-images.githubusercontent.com/44516223/134251742-2418618d-48dc-472b-872e-6b58956e8daa.png)

Solder male headers onto the Feather. Insert the header from the bottom of the sensor with the long side facing downward.

#### 5. Hypnos

![image](https://user-images.githubusercontent.com/44516223/134251825-b59701ae-adcc-4570-ae02-aec9f4d235e7.png)
![image](https://user-images.githubusercontent.com/44516223/134251840-fa802591-c07f-477d-a1f3-c4e7b49625dc.png)

         Front                                                                Back  
Solder 12-pin female headers upwards on the inner rail [Feather rail]  on the left of the Hypnos front and 16-pin female headers on the outer rail on the right side. Then, solder 12-pin male headers on the outer edge [sensor rail] of the left side, long side facing down. And 16-pin male headers on the inner rail, long side facing down.

#### 6. WeatherChimes PCB

![image](https://user-images.githubusercontent.com/44516223/134251927-f924096f-5f54-419d-b6ba-6bdb15f23923.png)
![image](https://user-images.githubusercontent.com/44516223/134251937-5f6565dc-1375-4303-a4c9-df8c40be2447.png)

Solder 3-pin JST connectors in the red box, and 4-pin JST connectors in the black boxes. Note the notches as they note how the connector should be oriented. Then solder male headers, inserted from the bottom of the board with the long side facing downward. 

#### 7. Waterproof 4-wire cable set

![image](https://user-images.githubusercontent.com/44516223/134251993-0dc7f2ad-ca57-48d6-84c8-7027feaefb9d.png)
![image](https://user-images.githubusercontent.com/44516223/134252004-ab9f42f9-78b7-4dcc-9694-ce403a0bd831.png)

The other end of the plug (female) is where the sensor is soldered. Note that the black cap should be slipped onto the cable gland before any soldering is done. The jack (male) should be slipped through the PG7 cable gland and connected to the 4-pin JST wires.


#### 8. Threaded hex standoffs

![image](https://user-images.githubusercontent.com/44516223/134252058-c50bc43d-86b9-444d-ab8f-17d364b4329a.png)

Screw the threaded end of the standoff to the base plate, and use 2.5mm bolts to screw the TSL2591 to the standoffs.

### Build Complete! Now to set up the software

## Setting Up HiveMQ Broker
Create a new HiveMQ Broker at this link https://console.hivemq.cloud/  
MQTT basics: https://www.hivemq.com/mqtt-essentials/
Testing connection to the broker: http://www.hivemq.com/demos/websocket-client/

## Setting Up MongoDB Database
MongoDB Manual: https://docs.mongodb.com/manual/  
**For OPEnS Lab Projects**: Communicate with [Chet Udell](mailto:udellc@oregonstate.edu?subject=[GitHub]%20Source%20Han%20Sans) to gain access to the OPEnS Lab MongoDB organization.  
**Other**: Create a new organization in MongoDB, https://mongodb.com  

Once you have access to an organization, create a project.  
In the project create cluster.  
After the cluster is created there will be a connect button.  
Follow the steps to create admin access to the database. 
After an admin account is made, chose connect to an application and from the drop down menu choose Node.js.  
Make sure the check box saying "Include full driver code example" is not checked.  
Copy the text from the "@" to before the second ".". This will be used in later steps referred to as the MongoDB unique cluster variable.  
- Example: ...@**examplecluster.3na0r**.mongodb.net/...

## Pass Through Script
The Pass Through Script needs be run on server for the duration of data collection for a project.  
To get the pass through script working properly you will have to install some external packages and change some variables within Pass.js pertaining to your project set-up.  
The first step is to install Node.js which can be installed here: https://nodejs.org/en/download/  
and node package manager (npm) which can be installed here: https://docs.npmjs.com/downloading-and-installing-node-js-and-npm  

After Node and npm is installed you will need to install using mqtt and mongodb node packages using the node package manager `npm`  
For mqtt: `npm install mqtt --save` https://www.npmjs.com/package/mqtt#install  
For mongodb: `npm install mongodb` https://www.w3schools.com/nodejs/nodejs_mongodb.asp  
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

The Pass through script works by connecting to the HiveMQ broker for you project and the MongoDB database you are storing data for your project. When ever a meassage is receved to the HiveMQ broker the message contents is passed into the `database()` function along with the second MQTT topic level. The `database()` function uses parses the message contents to send as the contents of a document in MongoDB and uses the passed topic level as the name of the collection the data will be stored under.

For the use in WeatherChimes, the first topic level to be the name of the database, and the second topic level is the name of the device and also the na,e of the collection withing the database on MongoDB. The data being sent through the HiveMQ database is in a format that does not need to be altered, so it is passed through and parses it into a json that MongoDB can read and store.


## MQTT Dirty Integration for Loom

WeatherChimes is currently using a dirty integration of the network protocol `MQTT`. Since it is has not been fully integrated into Loom, users need to ensure that the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) is included in their `.ino` project file. It has already been integrated into the [weathercomplete](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/weathercomplete.ino)`.ino` file.  

If you would like to use MQTT for a project that is not using Loom you can build off of examples from the [ArduinoMQTTClient Library](https://github.com/arduino-libraries/ArduinoMqttClient/tree/master/examples) here.

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

The MongoDB Connection Max Patch uses a Node.js script to connect to a database and recieve live updated information.  
To have the Node.js script running properly ensure that Node is installed. You can do that [here]( https://nodejs.org/en/download/).  
If this is your first time using the MongoDB Max Patch and Node is installed click the the `Setup` button on the patch to install the necessary packages to run the main script for the patch.  

The Setup button is installing the node package manager (npm) and the mongodb node package.   
NPM can also be installed here: https://docs.npmjs.com/downloading-and-installing-node-js-and-npm  
mongodb node package can be installed using the line `npm install --mongodb`  

### Using the Max Patch
After completing the setup the user can now use the Max Patch.

To use the MongoDB Connect Max Patch, the user must input four peices of information
1. MongoDB admin username
2. MongoDB admin password
3. MongDB unique cluster variable can be found here: https://github.com/OPEnSLab-OSU/WeatherChimes#setting-up-mongodb-database
4. The name of the database they would like to connect to, it should be the name of database in Pass through script. **Example: Chime**
5. The name of the device (collection) they would like to connect to. This is declared in the config file of the weathercomplete .ino **Example: Chime1**


After all of this information is typed into the corresponding boxes, click Send, which begins the script and connects to the MongoDB database

Data will be streamed out of the outlet of the patch, and can be used seamlessly with the Loom Sensor Max Patch to read data.

When connected successfully, if the inlet of the Max Patch recieves a bang it will output the most recent data that was sent to the connected collection. This also happens upon connecting when Send is clicked.

Further additions can be made to this Patch using the mongodb node package to get aggregate data outputs and much more. 
  - https://docs.mongodb.com/manual/
  - https://docs.mongodb.com/drivers/node/current/

