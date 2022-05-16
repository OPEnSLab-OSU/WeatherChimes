#pragma once
#include <ArduinoMqttClient.h>
#include <WiFi101.h>

#include "arduino_secrets.h"

// Feather M0 Wifi Pins
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char broker[] = SECRET_BROKER;
int port = BROKER_PORT;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

/* Disconnect from broker and WiFi*/
void disconnect_wifi(){
  mqttClient.stop();
  WiFi.disconnect();
  WiFi.end();
}

/**
 * Connect to the wifi network
 */ 
void connect_to_wifi(){

    // Try to conect to the wifi network
    Serial.print("[MQTT] Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Check if there is a password for the wifi or not
    if(strlen(pass) > 0){
        // Try to connect to the Wifi network until it succeeds 
        while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
            Serial.print("[MQTT] Attempting connection to AP...");
            delay(5000);
        }
    }
    else{
        // Try to connect to the Wifi network until it succeeds 
        while (WiFi.begin(ssid) != WL_CONNECTED) {
            Serial.print("[MQTT] Attempting connection to AP...");
            delay(5000);
        }
    }

    Serial.println("Connected to network!");
}

/*
 * Enable the Wifi but don't connect to any network
 */
void enable_wifi(){
  
  // Set the pins that the WiFi module should
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

/**
 * Connect to the MQTT broker
 */ 
void connect_to_broker(int keepAliveMins){

    // Set the MQTT username and password
    mqttClient.setUsernamePassword(BROKER_USER, BROKER_PASSWORD);

    // Set keep alive time
    mqttClient.setKeepAliveInterval(1000 * 60 * keepAliveMins);

    Serial.print("[MQTT] Attempting to connect to broker: ");
    Serial.println(broker);

    // Try to connect to the broker
    if (!mqttClient.connect(broker, port)) {
        Serial.print("[MQTT] MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());
    }

    Serial.println("[MQTT] You're connected to the MQTT broker!");
}

/**
 * Publish the data to the correct topic
 */ 
void publish_mqtt(String topic, String data){

    // send message, the Print interface can be used to set the message contents
    mqttClient.poll();
    mqttClient.beginMessage(topic);
    mqttClient.print(data);
    mqttClient.endMessage();
}
