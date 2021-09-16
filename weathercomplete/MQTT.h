
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoMqttClient.h>

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
/************************* Code *********************************/


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
    mqttClient.connect(broker, port);
    delay(3000);
  } 

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
  
}


void Wifi_setup(){
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  
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


JsonObject parse_for_send(JsonObject internal)
{
  
//JsonObject internal = Feather.internal_json(); 
  
  JsonArray arr = internal["contents"].as<JsonArray>(); //get the sensor data JSON
  Serial.println("begin test data");

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


Serial.println("End Parse function");
  return root; 
}
