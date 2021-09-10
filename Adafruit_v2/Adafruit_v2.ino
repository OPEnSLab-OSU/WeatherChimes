/***************************************************
  Adafruit MQTT Library WINC1500 Example

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>
#include <ArduinoJson.h>
#define LEDPIN 13


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
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

char ssid[] = SECRET_SSID;     //  your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    SECRET_USERNAME 
#define AIO_KEY         SECRET_KEY
/************ Global State (you don't need to change this!) ******************/

//Set up the wifi client
WiFiSSLClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish groupObj = Adafruit_MQTT_Publish(&mqtt, tempURL.c_str());

// Setup a feed called 'onoff' for subscribing to changes.
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");
//Adafruit_MQTT_Subscribe throttle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");
/*************************** Sketch Code ************************************/

void setup() {
//loom stuff
  Feather.begin_serial(true);
  Feather.parse_config(json_config);
  Feather.print_config();

  LPrintln("\n ** Setup Complete ** ");




//mqtt start
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
  
  pinMode(LEDPIN, OUTPUT);
}


void loop() {
  //get data from loom
  Feather.measure();
  Feather.package();
  Feather.display_data();

  char name[20];
  Feather.get_device_name(name);



  String groupName = String(name);
  groupName.toLowerCase();

//creates url to send to 
  char tempURL[50];
  snprintf(tempURL, 50, "%s%s%s", AIO_USERNAME, "/groups/", "rij4");
  //String tempURL = String(String(AIO_USERNAME) + "/groups/" + groupName + "/json");

//creates publish object
  Adafruit_MQTT_Publish groupObj = Adafruit_MQTT_Publish(&mqtt, tempURL, MQTT_QOS_0, 1);

  //finds json to be parsed
  JsonObject internal = Feather.internal_json();
  
  JsonArray arr = internal["contents"].as<JsonArray>();
  Serial.println("begin test data");


  //creates JSON TO BE PUBLISHED 
  DynamicJsonDocument JSONencoder(1024);
  JsonObject root = JSONencoder.to<JsonObject>();
  JsonObject feeds = root.createNestedObject("feeds");
  
//parses json
  for (JsonObject item: arr){
    if ((item["module"].as<char*>() != "Packet") && (String(item["module"].as<char*>()) != "Analog")){
      String modName = item["module"].as<char*>();
      JsonObject data = item["data"].as<JsonObject>();
      for (JsonPair kv : data) {
        String valName = kv.key().c_str();
        String feedName = (modName + "-" + valName);
        if(valName != "Full"){
          feeds[feedName] = kv.value().as<int>();
        }
      }
    }
  }
  //prints info to console for debugging
  Serial.println(groupName);
  Serial.println(tempURL);

  //converts JSON to c-string
  int sizeOfJSON = JSONencoder.capacity();
  char JSONmessageBuffer[sizeOfJSON];
  serializeJson(root, JSONmessageBuffer,sizeOfJSON); // needs 3rd parameter when using c -string
  
  //char buffer[1024];
  //serializeJson(root, buffer);
  serializeJsonPretty(root, Serial);
  //Serial.print("\n" buffer);
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop

  // Now we can publish stuff!
  Serial.print(F("\nSending groupObj val "));
//publishes the data
  Serial.print("...");
  if (! groupObj.publish(JSONmessageBuffer)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  Feather.pause();
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.

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
  if (mqtt.connected()) {
    return;
  }

  Serial.print("\nConnecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
