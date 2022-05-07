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

## MQTT Integration with Loom

Uploading the code to the Feather WiFi requires an USB cable. 
Download the code from the WeatherChimes Github repository and put WeatherChimes.ino, arduino_secrets.h and config.h in the same folder and open in Arduino.
Go to Tools >> Board >> Loom SAMD boards >> Loomified Feather M0. Then also check Tools >> Ports and see if the correct port has been selected and the board is appearing on said port. Upload the code to the Feather, after it has finished compiling, check to see if the upload was successful by opening the Arduino IDE serial monitor to see successful connections to the WiFi and MQTT broker as well as the packets of data being sent over the MQTT protocol. 

The system clock needs to be set on the first run or when the Hypnos coin cell battery is reset. 
Within the weatherchimes.ino, line 141 
`141 getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 10, 0));`
The sampling interval could be changed to any duration. From left to right, the numbers represent days, hours, minutes and seconds. 

The arduino_secrets.h file also needs to include the user’s WiFi name and password, as well as the MQTT settings. The SECRET_SSID and SECRET_PASS variables correspond to the WiFi router name and password. This WiFi router should be connected to the World Wide Web with no firewall settings that would restrict communications on the Broker Port. BROKER_USER and BROKER_PASSWORD correspond to the username and password set on the MQTT broker. The SECRET_BROKER is the server (IP Address / Hostname) where the MQTT Broker is listening. The BROKER_PORT is where that MQTT Broker is listening on the hostname. Finally, the SITE_NAME is not directly related to MQTT but rather the passthrough process as a whole, this tells the MongoDB server which database we should store the data in as it is passed along as the first level in the MQTT topic. 
```
2  // Wifi settings
3  #define SECRET_SSID "OSU_Access"
4  #define SECRET_PASS ""
5
6  // MQTT Settings
7  #define BROKER_USER "User"
8  #define BROKER_PASSWORD "password"
9  #define SECRET_BROKER "cas-mosquitto.biossys.oregonstate.edu"
10 #define BROKER_PORT 1883
11 #define SITE_NAME "WeatherChimes" //The name of the location where these  nodes will be placed
```


 The instance number in the config.h needs to be changed so it logs to the right collection. The interval value does not matter. 
 ```
1 {\
2  'general':\
3  {\
4    'name':'Chime',\   
5    'instance':1,\
6    'interval':2000\
7  },\
```
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



