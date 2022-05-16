///////////////////////////////////////////////////////////////////////////////

// This is a basic example that demonstrates usage of the Hypnos board
// Deep sleep functionality.

// The Hypnos board includes
// - SD
// - DS3231 RTC
// - Ability to power of peripherals

// Further details about the Hypnos board can be found here:
// https://github.com/OPEnSLab-OSU/OPEnS-Lab-Home/wiki/Hypnos

///////////////////////////////////////////////////////////////////////////////

#include <Loom.h>
#include "MQTT.h"

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

volatile bool rtc_flag = false;

void wakeISR_RTC() {
  // disable the interrupt
  detachInterrupt(12);
  rtc_flag = true;
}

void setup()
{
  // Needs to be done for Hypnos Board
  pinMode(5, OUTPUT);   // Enable control of 3.3V rail
  pinMode(6, OUTPUT);   // Enable control of 5V rail
  pinMode(12, INPUT_PULLUP);    // Enable waiting for RTC interrupt, MUST use a pullup since signal is active low
  pinMode(13, OUTPUT);

  // See Above
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail
  digitalWrite(13, LOW);

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
  Serial.flush();
}


void loop()
{
  digitalWrite(5, LOW); // Disable 3.3V rail
  digitalWrite(6, HIGH);  // Disable 5V rail
  digitalWrite(13, HIGH);

  // As it turns out, if the SD card is initialized and you change
  // the states of the pins to ANY VALUE, the SD card will fail to
  // write. As a result, we ensure that the board has been turned
  // off at least once before we make any changes to the pin states
  if (rtc_flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);

    // delay(1000);

    Feather.power_up();
  }

  Feather.measure();
  Feather.package();
  Feather.display_data();

  getSD(Feather).log();

  connect_to_wifi();
  connect_to_broker(1);

  doc.clear();
  jsonSerialized = "";
  doc.add(Feather.internal_json(false));
  serializeJson(doc, jsonSerialized);
  publish_mqtt(topic, jsonSerialized);


  // set the RTC alarm to a duration of 10 seconds with TimeSpan
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0,0,0,10));
  getInterruptManager(Feather).reconnect_interrupt(12);

  disconnect_wifi();

  digitalWrite(13, LOW);
  digitalWrite(5, HIGH); // Enable 3.3V rail
  digitalWrite(6, LOW);  // Enable 5V rail
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

  rtc_flag = false;
  Feather.power_down();
  getSleepManager(Feather).sleep();
  //Feather.pause();
  Feather.power_up();
  while (!rtc_flag);
}
