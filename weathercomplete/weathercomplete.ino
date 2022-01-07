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
char* ssid = SECRET_SSID;// your network SSID (name)
char* pass = SECRET_PASS;

char* broker = SECRET_BROKER; //OPEnS specific HiveMQ broker
int broker_port = BROKER_PORT;

char devName[20];
String topic = "";

String jsonResponse = "";
DynamicJsonDocument doc(1024);

volatile bool flag = false;   // Interrupt flag

void send_MQTT_data(){
  jsonResponse = "";
  doc.clear();

   // Get the internal JSON object of the data
  doc.add(Feather.internal_json(false));
  LMark;
  serializeJson(doc, jsonResponse);
  LMark;

  // Connect to WIFI and the MQTT Broker
  MQTT_connect(ssid, pass, broker, broker_port);
  LMark;

  // Poll the broker to avoid being disconnected by the server
  mqttClient.poll();
  LMark;

  mqttClient.beginMessage(topic);
  mqttClient.print(jsonResponse);
  mqttClient.endMessage();
}


// Interrupt Functions
void ISR_pin12()
{
  detachInterrupt(12);
  flag = true;
}

void setup()
{
  // Needs to be done for Hypnos Board
  pinMode(5, OUTPUT);   // Enable control of 3.3V rail
  pinMode(6, OUTPUT);   // Enable control of 5V rail
  pinMode(12, INPUT_PULLUP);    // Enable waiting for RTC interrupt, MUST use a pullup since signal is active low
  pinMode(13, OUTPUT);

  digitalWrite(5, LOW);  // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  digitalWrite(13, LOW);

  Feather.begin_serial(true);
  Feather.parse_config(json_config);
  Feather.print_config();

  getInterruptManager(Feather).register_ISR(12, ISR_pin12, LOW, ISR_Type::IMMEDIATE);

  Feather.get_device_name(devName);
  topic = String(SITE_NAME) + "/" + String(devName) + String(Feather.get_instance_num());

  LPrintln("Publishing Topic: ", topic);

  // Setup MQTT
  setup_MQTT();


  LPrintln("\n ** Setup Complete ** ");
}

void loop()
{

  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  digitalWrite(13, HIGH);
  
  if (flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(11, OUTPUT);
    
    Feather.power_up();
  }

  Feather.measure();
  Feather.package();
  Feather.display_data();

  getSD(Feather).log();
  
  send_MQTT_data();

  // Set the sleep time to 5 seconds
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 0, 10));
  getInterruptManager(Feather).reconnect_interrupt(12);

  digitalWrite(13, LOW);
  digitalWrite(5, HIGH); // Disable 3.3V rail
  digitalWrite(6, LOW);  // Disable 5V rail
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(11, INPUT);

  flag = false;
  Feather.power_down();
  getSleepManager(Feather).sleep();
  Feather.power_up();
  while (!flag); //waits for an interrupt flag

}
