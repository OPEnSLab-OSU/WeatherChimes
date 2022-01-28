///////////////////////////////////////////////////////////////////////////////
//
// This is the operational code for the Weatherchimes Project
// It uses Loom as a library to manage a TSL2591 luminosity sensor, and a SHT31D temperature and humidity sensor
// It also uses Loom to manage the Hypnos board for SD card logging capabilities
//
// Finally, apart from Loom, this project integrares functionality for a SDI12 Decagon GS3 sensor
// that measures conductivity, water content and temperature
//
///////////////////////////////////////////////////////////////////////////////

#include <ArduinoJson.h>
#include <Loom.h>
#include <String.h>
#include "SDI12.h"
#include "MQTT.h"
#include "arduino_secrets.h"

// Include configuration
const char* json_config =
#include "config.h"
;

// In Tools menu, set:
// Internet  > Disabled
// Sensors   > Enabled
// Radios    > Disabled
// Actuators > Disabled
// Max       > Disabled

using namespace Loom;

Loom::Manager Feather{};


/*************************** MQTT Settings *****************************/

// note: may cause problems if the ssid contains a space? behavior unsure
char* ssid = SECRET_SSID; // WiFi SSID Name
char* pass = SECRET_PASS; // WiFI Password

char* broker = SECRET_BROKER; // Address of our MQTT Broker
int broker_port = BROKER_PORT; // Port that our broker is listening on

/***********************************************************************/

// Buffer to store the name of the current chime device
char devName[20];

// Contains the name of the MQTT topic we will publish to
String topic = "";

// Stringified JSON that we want to send over MQTT
String jsonResponse = "";
DynamicJsonDocument doc(1024);

volatile bool RTC_Wake_Flag = false;   // Sleep interrupt flag, triggered with the RTC (Real-Time Clock) wants to wake the device up from sleep

/**
 * Transmits MQTT data to the specified broker on the constructed topic name
 */ 
void send_MQTT_data(){
  jsonResponse = "";
  doc.clear();

  // Get the internal packaged sensor data and add it to our JSON document
  doc.add(Feather.internal_json(false));

  // Convert the JSON dictionary to a string
  serializeJson(doc, jsonResponse);

  // Connect to WIFI and the MQTT Broker
  MQTT_connect(ssid, pass, broker, broker_port);

  // Poll the broker to avoid being disconnected by the server
  mqttClient.poll();

  // Start broadcasting a message on the given topic
  mqttClient.beginMessage(topic);

  // Write the message to the topic
  mqttClient.print(jsonResponse);

  // End the current message
  mqttClient.endMessage();
}


/** 
 * Triggered by the RTC when we want to wake the device up. 
 * This method should return as quickly as possible as it is an interrupt callback and not a logic processor.
 */
void RTC_Interrupt(){

  // Detach the interrupt on PIN 12 to avoid it triggering again while we are collecting data
  detachInterrupt(12);
  RTC_Wake_Flag = true;
}

/**
 * Called on initial power up/reset, initializes pins and libraries to begin collecting data
 */ 
void setup(){

  // Setting Pin 5 to Low enables the 3.3v Rail on the Hypnos
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  // Setting Pin 6 to High enables the 5v Rail on the Hypnos
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);

  // Enables the builtin led to show the status of the Feather
  Feather.begin_LED();

  // Opens a serial connection for writing debug information
  Feather.begin_serial(true);

  // Parses the component config to decide what sensors to create
  Feather.parse_config(json_config);

  // Print the config to the serial so we know what config we are using
  Feather.print_config();

  // Schedule the wake interrupt on pin 12, the RTC interrupt is a logic low interrupt so when pin 12 flips from high to low the interrupt is triggered
  getInterruptManager(Feather).register_ISR(12, RTC_Interrupt, LOW, ISR_Type::IMMEDIATE);

  // Read the device name from the manager and write it into the buffer we made earlier
  Feather.get_device_name(devName);

  // Create the topic we are publishing to by starting with the SITE_NAME, where the devices are located followed by a combination of the device name with the device instance append on
  topic = String(SITE_NAME) + "/" + String(devName) + String(Feather.get_instance_num());

  // Inform the user what topic we are publishing to
  LPrintln("Publishing Topic: ", topic);

  // Sets up the MQTT connection (See MQTT.h)
  setup_MQTT();

  LPrintln("\n ** Setup Complete ** ");
}

void loop()
{

  // Initialize Hypnos
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail

  // Check if we are being woken up from a sleep vs. just a normal run through
  if (RTC_Wake_Flag) {

    // Re-enable the SPI interface and SD card pin, call power_up again so that the SD card reinitializes
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);

    Feather.power_up();
  }

  // Request a measure on the sensors
  Feather.measure();

  // Take the data previously measured from the sensors and package it into a JSON document
  Feather.package();

  // Print the data to the Serial bus
  Feather.display_data();

  // Log the data to the connected SD card
  getSD(Feather).log();

  // Transmits the data over MQTT to the broker (See above)
  send_MQTT_data();

  // Reset the RTC alarm to fire 10 mins from now, and then reconnect the interrupt so future interrupts can be triggered
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 10, 0));
  getInterruptManager(Feather).reconnect_interrupt(12);

  // Set the wake flag back to false so we can sleep again
  RTC_Wake_Flag = false;

  // Prepare modules for sleeping
  Feather.power_down();

  // Disable SPI and SD card, by setting the mode to input
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

  // Power down the power rails on the Hypnos
  digitalWrite(5, HIGH); // Disable the 3.3v rail
  digitalWrite(6, LOW); //  Disable the 5v rail

  // Finally tell the feather to completely shut down and wait for an interupt
  getSleepManager(Feather).sleep();

  // Wait for the interrupt flag to turn true so we can break out of the loop and run our code again
  while (!RTC_Wake_Flag);

}