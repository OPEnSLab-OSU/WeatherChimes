
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
  Serial.println("MQTT: Line 33");

  // Put the wifi chip in max power saver mode
  //WiFi.maxLowPowerMode();

  LMark;

  Serial.println("Line 40");
  delay(2000);
  Serial.println("Line 42");
  // Connect to WIFI given the creds
  while(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting to Access Point: ");
      Serial.println(ssid);

      LMark;

      // Check if the AP has a password
      if(strlen(pass) > 0){
        WiFi.begin(ssid, pass);
      }
      else{
        WiFi.begin(ssid);
      }

      Serial.println("MQTT: Line 56");
      
      
      // wait 10 seconds for connection:
      uint8_t timeout = 14;
      while (timeout && (WiFi.status() != WL_CONNECTED)) {
        Serial.println("Line 62");
        timeout--;
        delay(1000);
      }
  }

  Serial.println("MQTT: Line 67");
  if (mqttClient.connected()) {return;}

  Serial.println("MQTT: Line 71");
  mqttClient.setUsernamePassword(BROKER_USER, BROKER_PASSWORD);

  // Print a succcsess message and the device's IP
  Serial.println("Connected to Network!");
  Serial.print("Device IP: ");
  Serial.println(IPAddress(WiFi.localIP()));

  
  // Announce we are connecting to the MQTT broker
  Serial.print("Connecting to MQTT Broker: ");
  Serial.println(broker);

  //mqttClient.setId("WeatherChimes-123");

  LMark;
  if(!mqttClient.connect(broker, broker_port)){
    Serial.print("Connection Error Occurred: ");
    Serial.println(mqttClient.connectError());
    LMark;
  }
  else{
    LMark;
    Serial.println("Connected to the MQTT Broker!");
  }

  
}

void disconnect_wifi(){
  WiFi.disconnect();
}

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
