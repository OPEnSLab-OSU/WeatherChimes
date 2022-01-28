
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
  // Put the wifi chip in max power saver mode
  WiFi.maxLowPowerMode();

  // Connect to WIFI given the creds
  while(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting to Access Point: ");
      Serial.println(ssid);

      // Check if the AP has a password, to decicde which method to call
      if(strlen(pass) > 0){
        WiFi.begin(ssid, pass);
      }
      else{
        WiFi.begin(ssid);
      }

      
      
      // While we arent connected retry the connection every 10 seconds 
      uint8_t timeout = 10;
      while (timeout && (WiFi.status() != WL_CONNECTED)) {
        Serial.println("Line 62");
        timeout--;
        delay(1000);
      }
  }

  // If we are already connected to the MQTT broker simply return to save time
  if (mqttClient.connected()) {return;}

  // Set the brokers Username and Passowrd
  mqttClient.setUsernamePassword(BROKER_USER, BROKER_PASSWORD);

  // At this point we will have connected to the network so print out our IP address on the network
  Serial.println("Connected to Network!");
  LPrintln("Device IP: ", IPAddress(WiFi.localIP()))

  // Announce we are connecting to the MQTT broker
  Serial.print("Connecting to MQTT Broker: ");
  Serial.println(broker);
  
  // Try to connect to the given broker with the given credentials, if it fails tell us there was a connection error
  if(!mqttClient.connect(broker, broker_port)){
    Serial.print("Connection Error Occurred: ");
    Serial.println(mqttClient.connectError());
  }

  // If no error occurred then we succsessfully connected to the broker
  else{
    Serial.println("Connected to the MQTT Broker!");
  }
}

/**
 * Disconnects from the current wifi network
 */ 
void disconnect_wifi(){
  WiFi.disconnect();
}

/**
 * Setup the WiFi pins an check if the current Fether M0 actually has a WiFi shield
 */ 
void setup_MQTT(){
  
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
