///////////////////////////////////////////////////////////////////////////////
//
// This is the operational code for the Weatherchimes Project
// It uses Loom as a library to manage a TSL2591 luminosity sensor, SHT31D temperature and humidity sensor, and the GS3 Decagon Soil Moisture, Temperature and Conductivity sensor
// It also uses Loom to manage the Hypnos board for SD card logging and sleep capabilities
//
///////////////////////////////////////////////////////////////////////////////

// In Tools menu, set:
// Internet  > Disabled
// Sensors   > Enabled
// Radios    > Disabled
// Actuators > Disabled
// Max       > Disabled

#include <ArduinoJson.h>
#include <Loom.h>
#include <String.h>
#include "MQTT.h"
#include "arduino_secrets.h"

// Include configuration
const char* json_config =
#include "config.h"
;

using namespace Loom;

Loom::Manager Feather{};


/*************************** MQTT Settings *****************************/

char* ssid = SECRET_SSID;// your network SSID (name)
char* pass = SECRET_PASS;

char* broker = SECRET_BROKER; //OPEnS MQTT Broker, for all in lab projects the broker is run on an OSU server using Mosquitto
int broker_port = BROKER_PORT; // Port the broker is listening on for connections

// Buffer to hold the name of the device and an empty string to constuct the publishing topic in
char devName[20];
String topic = "";

// Where to store the serialized loom JSON packet to send over MQTT
String jsonResponse = "";
DynamicJsonDocument doc(1024);

volatile bool flag = false;   // Interrupt flag

void send_MQTT_data(){

  // Reset the response and clear the document
  jsonResponse = "";
  doc.clear();

  // Get the JSON data from loom and store it in the document locally
  doc.add(Feather.internal_json(false));

  // Serialize the JSON document into a string
  serializeJson(doc, jsonResponse);

  // Poll the broker to avoid being disconnected by the server
  mqttClient.poll();

  // Start a new message on the constructed topic and then send the JSON data over the channel closing it when completed
  mqttClient.beginMessage(topic);
  mqttClient.print(jsonResponse);
  mqttClient.endMessage();
}


// Interrupt is called by the RTC on the Hypnos board, wakes up informs the loop that it should execute disconnecting from the interrupt so as to not trigger another while code is executing
void ISR_pin12()
{
  detachInterrupt(12);
  flag = true;
}

void setup()
{
  pinMode(5, OUTPUT); // Enable the 3.3v pin for writing
  pinMode(6, OUTPUT); // Enable the 5v pin for writing
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  Feather.begin_LED();  // Set pin 13 to Output so the LED can be used
  Feather.begin_serial(true); // Start the serial connection waiting 20 seconds for the user to open a serial connection before proceeding
  Feather.parse_config(json_config);  // Parse the JSON config
  Feather.print_config(); // Print out the parsed config

  // Register the wake-up interrupt on pin 12
  getInterruptManager(Feather).register_ISR(12,ISR_pin12, LOW, ISR_Type::IMMEDIATE);

  // Get the device name and then using the device name, instance number and site name create a topic name to publish to
  Feather.get_device_name(devName);
  topic = String(SITE_NAME) + "/" + String(devName) + String(Feather.get_instance_num());

  // Print out the topic name
  LPrintln("Publishing Topic: ", topic);

  // Making sure we can actually use WiFi and setting the WiFi chip to low power mode
  setup_MQTT();

  // Connect to WIFI and the MQTT Broker 
  MQTT_connect(ssid, pass, broker, broker_port);


  LPrintln("\n ** Setup Complete ** ");
}

void loop()
{

  // Initialize Hypnos
  digitalWrite(5, LOW); // Enable 3.3V rail
  digitalWrite(6, HIGH);  // Enable 5V rail

  // Only executed after waking up from sleep, reinitilizes the SPI pins as well as the SD card chip select
  if (flag) {
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(10, OUTPUT);

    Feather.power_up();
  }
  
  // Polls all the sensors for measurements and then packages the data into a JSON document
  Feather.measure();
  Feather.package();

   // Add RSSI Values to packet
  Feather.add_data("WiFi", "WiFi", WiFi.RSSI());

  // Print the data out to the serial bus
  Feather.display_data();

  // Log the data to the SD card
  getSD(Feather).log();

  // Serialize and transmit the data over MQTT to our remote broker
  send_MQTT_data();

  // Set the interrupt alarm on the RTC to 5 minutes in the future, reconnecting to the interrupt so we can wake up again
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 0, 10));
  getInterruptManager(Feather).reconnect_interrupt(12);

  // Power down modules
  Feather.power_down();


  // By setting the SPI pins and SD chip select to Input we effectively stop transmitting over them, thus saving battery
  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

  // Protocol to turn off Hypnos
  digitalWrite(5, HIGH); // Disabling all pins before going to sleep.
  digitalWrite(6, LOW);

  // Enter sleep and wait for the interrupt to be triggered
  getSleepManager(Feather).sleep();
  
  // Waits for an interrupt to trigger
  while (!flag); 

}
