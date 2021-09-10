///////////////////////////////////////////////////////////////////////////////
// 
// This is the operational code for the Weatherchimes Project
// It uses Loom as a library to manage a TSL2591 luminosity sensor, and a SHT31D temperature and humidity sensor
// It also uses Loom to manage the Hypnos board for SD card logging capabilities
// 
// Apart from Loom, this project integrares functionality for a SDI12 Decagon GS3 sensor
// that measures conductivity, water content and temperature
// 
// This file also connects to a local WiFi server via Loom and manually. This is a temporary solution for implementing
// MQTT connectivity to the WeatherChimes Project before it is implemented into Loom as a Publishing Platform.
// 
///////////////////////////////////////////////////////////////////////////////

  //INcluded in Loom
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoJson.h>
#include <Loom.h>
#include <String.h>

  //Not included in Loom yet
#include <ArduinoMqttClient.h>
#include "SDI12.h"

//Configuration file
const char* json_config =
#include "config.h"
;

// In Tools menu, set:
// Internet  > WiFi
// Sensors   > Enabled
// Radios    > Disabled
// Actuators > Disabled
// Max       > Disabled

using namespace Loom;

Loom::Manager Feather{};


/************************* WiFI Setup *****************************/

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC


//Need to declare these prior to running code!
char ssid[] = "NETGEAR33";    // your network SSID (name)
char pass[] = "tinybolt321";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

/************ Global State (you don't need to change this!) ******************/
WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

/************************* MQTT Broker Setup *********************************/
                     //OPEnS specific HiveMQ broker
const char broker[] = "8acfd6649bcd41f888ba886f128790ae.s1.eu.hivemq.cloud";   // public HiveMQ broker "broker.hivemq.com";
int        port     = 8883;      //Secure Port for MQTT, 1883 may not work with HiveMQ, need to test
String     topic  = "";      //MQTT topic that is being subcribed to in Pass-through Node.js file
String     HiveMQ_username = "WeatherChimes";
String     HiveMQ_password = "B1gchime";
#define DATAPIN 11         // change to the proper pin in which SDI12 is connected to on the Feather M0
SDI12 mySDI12(DATAPIN);

/************************* Code *********************************/

String sdiResponse = "";
String myCommand = "";
char   buf[20];
char   *p;
float  dielec_p = 0, temp = 0, elec_c = 0;
char sensor_address = 0;


// tokenize the sdiResponse into the three meaningful float data measurements
void parse_results()
{
  sdiResponse.toCharArray(buf, sizeof(buf));
  p = buf;

  strtok_r(p, "+", &p);
  dielec_p = atof(strtok_r(NULL, "+", &p));
  temp     = atof(strtok_r(NULL, "+", &p));
  elec_c   = atof(strtok_r(NULL, "+", &p));

   // LPrintln("== wcv: ",dielec_p);
   // LPrintln("== tmp: ",temp);
   // LPrintln("== elc: ",elec_c);
}


// reads the entire available sensor buffer (may read multiple messages)
void read_buffer()
{
  sdiResponse = "";
  delay(100);
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      sdiResponse += "<LF>";
    } else if (c == '\r') {
      sdiResponse += "<CR>";
    } else {
      sdiResponse += c;
    }
    delay(100);
  }
  // LPrintln("=== Buffer: ",sdiResponse); //write the response to the screen
  mySDI12.clearBuffer();
}


// reads just the next message in the buffer, stopping at CRLF
void read_next_message()
{
  sdiResponse = "";
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      break;
    }
    sdiResponse += c;
    delay(100);
  }
  if (sdiResponse[sdiResponse.length()-1] == '\r') {
    sdiResponse[sdiResponse.length()-1] = '\0';
  }
  // LPrintln("=== Message: ",sdiResponse); //write the response to the screen
}


// broadcast query to SDI12 sensors, set sensor_address to the first response
void get_address()
{
  mySDI12.clearBuffer();
  
  myCommand = "?!";
  mySDI12.sendCommand(myCommand);
  // LPrintln("\n=== Sending command: ",myCommand);
  delay(1000);
  read_buffer();
  sensor_address = sdiResponse[0];
  // LPrintln("address response: ", sdiResponse);
  // LPrintln("address as an int:", int(sensor_address));
  // LPrintln("address set to: ", sensor_address);
}


// sends a measure command and then a data request command
// Sets SDIResponse to the response message (which should contain the data)
void measure_decagon()
{
  // First command to take a measurement
  myCommand = String(sensor_address) + "M!";
  mySDI12.sendCommand(myCommand);
  delay(100);                     // wait a while for a response

  // the sensor will respond with some info which can be ignored here
  // the info includes the number of measurements to expect and time to wait
  // for future improvements, using this response could be useful
  read_buffer();
  mySDI12.clearBuffer();

  // next command to request data from last measurement
  myCommand = String(sensor_address) + "D0!";
  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  read_next_message();
  // if the sensor sent a service request before measurement data;
  // skip it and try for the next message, which should be the data
  if (sdiResponse.length() <= 3) {
    delay(30);
    read_next_message();
  }

  mySDI12.clearBuffer();
}


void package_decagon(){
    Feather.add_data("Decagon_GS3_M","moisture",dielec_p);
    Feather.add_data("Decagon_GS3_T","temperature",temp);
    Feather.add_data("Decagon_GS3_E","conductivity",elec_c);
}

void MQTT_connect() {
  int8_t ret;

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      delay(1000);
    }
  }
  
  // Stop if already connected.
  if (mqttClient.connected()) {
  return;
  }
  
  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  //mqttClient.setId("clientId-LVLkW4KQou");
  
  mqttClient.setUsernamePassword(HiveMQ_username, HiveMQ_password);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  while (!mqttClient.connect(broker, port))
 {
    // failed, retry
    Serial.println("Failed to connect to HiveMQ, Retrying");
    Serial.print(".");
    Serial.println(mqttClient.connectError());
    delay(3000);
  } 

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  
}

JsonObject parse_for_send(JsonObject internal)
{
  
//JsonObject internal = Feather.internal_json(); 
  
  JsonArray arr = internal["contents"].as<JsonArray>(); //get the sensor data JSON
  Serial.println("begin test data");

  MQTT_connect(); //connect to the MQTT client
  mqttClient.poll();
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker

  //creates JSON TO BE PUBLISHED 
  DynamicJsonDocument JSONencoder(1024); //JSON document to be written on
  JsonObject root = JSONencoder.to<JsonObject>(); //location of data in doc
 
//parses json
  for (JsonObject item: arr){
    //choose what data will be sent to the MQTT broker from the Loom contents array
    if ((item["module"].as<char*>() != "Packet") && (String(item["module"].as<char*>()) != "Analog")){
      String modName = item["module"].as<char*>();
      JsonObject modObj = root.createNestedObject(modName);
      JsonObject data = item["data"].as<JsonObject>();
      for (JsonPair kv : data) { 
        //create a nested item in the json that is the name of the sensor 
        //with key value pairs of the data the sensor is collecting 
        String valName = kv.key().c_str();
        if(valName != "Full"){
          modObj[valName] = kv.value().as<int>();
        }
      }
    }
  }

  //converts JSON to c-string
  int sizeOfJSON = 1024; // size of contents + padding
  char JSONmessageBuffer[sizeOfJSON];
  serializeJson(root, JSONmessageBuffer,sizeOfJSON); // needs 3rd parameter when using c -string
  serializeJsonPretty(root, Serial);

  Serial.println("End Parse function");
  return root; 
}

void setup()
{
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  
  // initialize decagon sensor, get its address, send first command
  mySDI12.begin();
  delay(2000);
  get_address();
  myCommand = String(sensor_address) + "I!";
  mySDI12.sendCommand(myCommand);
  delay(30);

  // read the decagon response, clear decagon buffer
  read_buffer();
  delay(30);
  mySDI12.clearBuffer();
  

  // Loom setup
  Feather.begin_LED();
  Feather.begin_serial(true,true);
  Feather.parse_config(json_config);
  Feather.print_config();

  LPrintln("\n ** Setup Complete ** ");

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

  while (!Serial);
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo for WINC1500"));

  // Initialise the Client
  Serial.print(F("\nInit the WiFi module..."));
  // check for the presence of the breakout
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WINC1500 not present");
    // don't continue:
    while (true);
  }
  Serial.println("ATWINC OK!");
}

void loop()
{
  
  Serial.println("In Loop");
  
  //Loom Processes
  
  Feather.measure();
  Feather.package();

  
  measure_decagon();
  parse_results();
  package_decagon();
  

  Feather.display_data();
  Feather.get<Loom::SD>()->log();

  
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

  //display data
    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print(JSONmessageBuffer);


    
  // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(JSONmessageBuffer); //correctly formatted data
    mqttClient.endMessage();

    Serial.println();

    Feather.pause();
}
