///////////////////////////////////////////////////////////////////////////////

// This is a basic example that demonstrates how to log data to an SD card
// using Loom.

///////////////////////////////////////////////////////////////////////////////

#include <Loom.h>
#include "MQTT.h"

// These are default times to sleep between cycles if there is no SD_config.txt file exists on SD card for user-set time
int secs = 20;
int mins = 0;
int hours = 0;
int days = 0;

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

char devName[20];
String topic = "";
String jsonSerialized = "";

DynamicJsonDocument doc(1024);

void setup()
{
  delay(3000); // Wait a bit in case we want to program the device
  pinMode(5, OUTPUT);   // Enable control of 3.3V rail 
  pinMode(6, OUTPUT);   // Enable control of 5V rail 

  //See Above
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  
  Feather.begin_LED();
  Feather.begin_serial(true);
  Feather.parse_config(json_config);
  Feather.print_config();

  // Register an interrupt on the RTC alarm pin
  getInterruptManager(Feather).register_ISR(12, wakeISR_RTC, LOW, ISR_Type::IMMEDIATE);

  // Get the device name and then using the device name, instance number and site name create a topic name to publish to
  Feather.get_device_name(devName);
  topic = String(SITE_NAME) + "/" + String(devName) + String(Feather.get_instance_num());

  enable_wifi();

  LPrintln("\n ** Setup Complete ** ");
}


void loop()
{
  Feather.measure();
  Feather.package();
  Feather.display_data();

  // Log using default filename as provided in configuration
  // in this case, 'datafile.csv'
  getSD(Feather).log();

  // Or log to a specific file (does not change what default file is set to)
  // getSD(Feather)log("specific.csv");

  // SD Hard Faults and makes the MQTT json packet change topic name to Errors#
  // So we use a customized function to build the correct topic and instance name
  // Log online to MongoDB via MQTT WiFi
  connect_to_wifi();
  connect_to_broker(1);
  // Build JSON document to publish via MQTT
  doc.clear();
  jsonSerialized = "";
  doc.add(Feather.internal_json(false));
  serializeJson(doc, jsonSerialized);
  //publish_mqtt(topic, jsonSerialized);
  publish_mqtt(String(SITE_NAME)+"/Chime"+String(Feather.get_instance_num()), jsonSerialized); // Replace content in Quotes with the Topic name

  // Set RTC Timer for wake up and sample to next time interval
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(days,hours,mins,secs));
  getInterruptManager(Feather).reconnect_interrupt(12);

  disconnect_wifi(); // Disable WiFi for power savings

  digitalWrite(5, HIGH); // Disable 3.3V rail
  digitalWrite(6, LOW);  // Disable 5V rail

  // Disable SPI pins/SD chip select to save power
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);  // Needs to be correct pin for SD CS on Hypnos

  Feather.power_down();
  getSleepManager(Feather).sleep(); // Sketch pauses here until RTC alarm

  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail

  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  pinMode(10, OUTPUT); // Needs to be correct pin for SD CS on Hypnos

  // *** SITS Here in Sleep till p12 RTC Alarm Wake Signal ...

  Feather.power_up();
  delay(1000);        // Delay for power 
}

void wakeISR_RTC() {  
  // disable the interrupt
  detachInterrupt(12);
}
