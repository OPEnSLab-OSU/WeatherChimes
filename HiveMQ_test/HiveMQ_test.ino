/*
  ArduinoMqttClient - WiFi Simple Sender

  This example connects to a MQTT broker and publishes a message to
  a topic once a second.

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev2 board

  This example code is in the public domain.
*****************************************************/
#include <SPI.h>
#include <ArduinoMqttClient.h>
#include <WiFi101.h>
#include <ArduinoJson.h>

#include <Loom.h>

// Include configuration
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
//#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

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
const char broker[] = "8acfd6649bcd41f888ba886f128790ae.s1.eu.hivemq.cloud";  //"broker.hivemq.com";
int        port     = 8883;
const char topic[]  = "arduino/data";

/************************* Code *********************************/
const long interval = 2000;
unsigned long previousMillis = 0;

int count = 0;

void setup() {
  //loom stuff
  Feather.begin_serial(true);
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
  
  //pinMode(LEDPIN, OUTPUT);

 

  
}

void loop() {
  //get data from loom
  Feather.measure();
  Feather.package();
  Feather.display_data();

  char name[20];
  Feather.get_device_name(name);

  String groupName = String(name + String(Feather.get_instance_num()));
  groupName.toLowerCase();


//finds json to be parsed
  JsonObject internal = Feather.internal_json();
  
  JsonArray arr = internal["contents"].as<JsonArray>();
  Serial.println("begin test data");


  MQTT_connect();
  mqttClient.poll();

  //creates JSON TO BE PUBLISHED 
  DynamicJsonDocument JSONencoder(1024);
  JsonObject root = JSONencoder.to<JsonObject>();
  //JsonObject feeds = root.createNestedObject("feeds");
 
//parses json
  for (JsonObject item: arr){
    if ((item["module"].as<char*>() != "Packet") && (String(item["module"].as<char*>()) != "Analog")){
      String modName = item["module"].as<char*>();
      JsonObject modObj = root.createNestedObject(modName);
      JsonObject data = item["data"].as<JsonObject>();
      for (JsonPair kv : data) {
        String valName = kv.key().c_str();
        if(valName != "Full"){
          modObj[valName] = kv.value().as<int>();
        }
      }
    }
  }

  Serial.println(groupName);

  //converts JSON to c-string
  int sizeOfJSON = JSONencoder.capacity();
  char JSONmessageBuffer[sizeOfJSON];
  serializeJson(root, JSONmessageBuffer,sizeOfJSON); // needs 3rd parameter when using c -string
  serializeJsonPretty(root, Serial);
  
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();
  

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.print(JSONmessageBuffer);
    //Serial.print("hello ");
    //Serial.println(count);


    
    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(JSONmessageBuffer);
    //mqttClient.print("hello ");
    //mqttClient.print(count);
    mqttClient.endMessage();

    Serial.println();

  Feather.pause();
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

  // You can provide a username and password for authentication
   mqttClient.setUsernamePassword("WeatherChimes", "B1gchime");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}
