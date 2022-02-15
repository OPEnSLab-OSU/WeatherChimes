
#include <SPI.h>
#include <WiFi101.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"
#include <string.h>

/************************* WiFI Setup *****************************/

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC

int status = WL_IDLE_STATUS;

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

/************ Global State (you don't need to change this!) ******************/
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

/************************* Code *********************************/

/**
 * Connect to WIFI and the MQTT broker
 */
void MQTT_connect(char* ssid, char* pass, char* broker, int broker_port){

  // Connect to WIFI given the creds
  while(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting to Access Point: ");
      Serial.println(ssid);

      // Check if the AP has a password
      if(strlen(pass) > 0){
        WiFi.begin(ssid, pass);
      }
      else{
        WiFi.begin(ssid);
      }
      
      
      // wait 10 seconds for connection:
      uint8_t timeout = 10;
      while (timeout && (WiFi.status() != WL_CONNECTED)) {
        timeout--;
        delay(1000);
      }
  }

  // Check if we are already connected to the broker and if so dont try to renegotiate a connection simply return
  if (mqttClient.connected()) {return;}

  // Set the MQTT broker Username and Password to use
  mqttClient.setUsernamePassword(BROKER_USER, BROKER_PASSWORD);

  // Set the keep alive time to be 6 minutes
  mqttClient.setKeepAliveInterval(1000 * 60 * 6);

  // Print a succcsess message and the device's IP
  Serial.println("Connected to Network!");
  Serial.print("Device IP: ");
  Serial.println(IPAddress(WiFi.localIP()));

  
  // Announce we are connecting to the MQTT broker
  Serial.print("Connecting to MQTT Broker: ");
  Serial.println(broker);

  // Attempt to connect to the broker with the given parameters printing the error code if it fails
  if(!mqttClient.connect(broker, broker_port)){
    Serial.print("Connection Error Occurred: ");
    Serial.println(mqttClient.connectError());
    return;
  }

  // Print out that our connection attempt was successful 
  Serial.println("Connected to the MQTT Broker!");
}

/**
 * Setup our WiFi chip 
 */
void setup_MQTT(){
  
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  
  // Initialise the Client
  Serial.print(F("\nInit the WiFi module..."));
  
  // Check for the presence of the breakout
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WINC1500 not present");
    
    // don't continue:
    while (true);
  }

  // Set the WiFi chip into the lowest power mode to conserve energy
  WiFi.maxLowPowerMode();
  
  Serial.println("ATWINC OK!");
  
}
