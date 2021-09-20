# WeatherChimes

## Setting Up HiveMQ Broker

## MQTT Dirty Integration for Loom

WeatherChimes is currently using a dirty integration of the network protocol `MQTT`. Since it is has not been fully integrated into Loom, users need to ensure that the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) is included in their `.ino` project file. It has already been integrated into the [weathercomplete](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/weathercomplete.ino)`.ino` file.  

If you would like to use MQTT for a project that is not using Loom you can build off of examples from the [ArduinoMQTTClient Library](https://github.com/arduino-libraries/ArduinoMqttClient/tree/master/examples) here.

### Here are the steps for integrating the [MQTT.h file](https://github.com/OPEnSLab-OSU/WeatherChimes/blob/main/weathercomplete/MQTT.h) into any Loom project. 

1. Put in your WiFi SSID and Pasword into the arduino_Secrets.h file
2. Set-up or gain access to a HiveMQ broker and put username and password into the appropriate line in arduino_secrets.h
3. Make sure that the MQTT.h file is in your project folder.
4. Have the line `#include "MQTT.h"` at the top of your main file `.ino` file
5. Before the Loop and Setup functions in your main `.ino` file put in this MQTT_send function:

> void MQTT_send(){  
> //Finds json to be parsed  
>  JsonObject internal = Feather.internal_json();  
> 
>  JsonObject all_sensor_data = parse_for_send(internal);  
>   
>  int sizeOfJSON = 1024; //JSONencoder.capacity();  
>  char JSONmessageBuffer[sizeOfJSON];  
>  serializeJson(all_sensor_data, JSONmessageBuffer,sizeOfJSON); // needs 3rd parameter when using c -string  
>  serializeJsonPretty(all_sensor_data, Serial);  
>  
>  //Serial.print(all_sensor_data);
>  
>  char name[20];  
>  Feather.get_device_name(name); // "name" will be one topic level that the data is published to on HiveMQ  
>  String groupName = String(name + String(Feather.get_instance_num()));  
>  String topic = String(String("Chime/") + groupName + String("/data"));  
>
>  MQTT_connect(); //connect to the MQTT client  
>  mqttClient.poll();  
>  // call poll() regularly to allow the library to send MQTT keep alives which  
>  // avoids being disconnected by the broker  
>
>  //display data  
>   Serial.print("Sending message to topic: ");  
>    Serial.println(topic);  
>    Serial.print(JSONmessageBuffer);  
>    
>  // send message, the Print interface can be used to set the message contents  
>    mqttClient.beginMessage(topic);  
>    mqttClient.print(JSONmessageBuffer); //correctly formatted data  
>    mqttClient.endMessage();  
>
> }

5. In your `Loop` function call `MQTT_send()` using the line: `MQTT_send();`
