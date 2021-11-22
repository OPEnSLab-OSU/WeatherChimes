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
  serializeJson(doc, jsonResponse);

  // Connect to WIFI and the MQTT Broker
  MQTT_connect(ssid, pass, broker, broker_port);

  // Poll the broker to avoid being disconnected by the server
  mqttClient.poll();

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
  while(!Serial);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  Feather.begin_LED();
  Feather.begin_serial(true);
  Feather.parse_config(json_config);
  Feather.print_config();

  getInterruptManager(Feather).register_ISR(12,ISR_pin12, LOW, ISR_Type::IMMEDIATE);

  Feather.get_device_name(devName);
  topic = String(SITE_NAME) + "/" + String(devName) + String(Feather.get_instance_num());

  LPrintln("Publishing Topic: ", topic);

  // Setup MQTT
  setup_MQTT();


  LPrintln("\n ** Setup Complete ** ");
}

void loop()
{

  // Initialize Hypnos
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail

  if (flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);

    Feather.power_up();
  }

  Feather.measure();
  Feather.package();

  Feather.display_data();
  getSD(Feather).log();
  send_MQTT_data();

  // Set the sleep time to 5 seconds
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 0, 5));
  getInterruptManager(Feather).reconnect_interrupt(12);

  //disconnect_wifi();

  Feather.power_down();

  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

  // Protocol to turn off Hypnos
  digitalWrite(5, HIGH); // Disabling all pins before going to sleep.
  digitalWrite(6, LOW);

  getSleepManager(Feather).sleep();
  while (!flag); //waits for an interrupt flag

}
