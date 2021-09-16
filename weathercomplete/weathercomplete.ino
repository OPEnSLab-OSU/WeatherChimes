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

#include <Loom.h>
#include <String.h>
#include "SDI12.h"

// Include configuration
const char* json_config =
#include "config.h"
;

// In Tools menu, set:
// Internet  > Enabled
// Sensors   > Enabled
// Radios    > Disabled
// Actuators > Disabled
// Max       > Disabled

using namespace Loom;

Loom::Manager Feather{};



#define DATAPIN 11         // change to the proper pin

SDI12 mySDI12(DATAPIN);

String sdiResponse = "";
String myCommand = "";
char   buf[20];
char   *p;
float  dielec_p = 0, temp = 0, elec_c = 0;
char sensor_address = -1;

volatile bool flag = false;   // Interrupt flag

// tokenize the Decagon sdiResponse into the three meaningful float data measurements
void parse_results()
{
  sdiResponse.toCharArray(buf, sizeof(buf));
  p = buf;

  strtok_r(p, "+", &p);
  dielec_p = atof(strtok_r(NULL, "+", &p));
  temp     = atof(strtok_r(NULL, "+", &p));
  elec_c   = atof(strtok_r(NULL, "+", &p));

   // LPrintln("== wcv: ",dielec_p);
   // LPrintln("== tmp: ",temp);
   // LPrintln("== elc: ",elec_c);
}


// reads the entire available Decagon buffer (may read multiple messages)
void read_buffer()
{
  sdiResponse = "";
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      sdiResponse += "<LF>";
    } else if (c == '\r') {
      sdiResponse += "<CR>";
    } else {
      sdiResponse += c;
    }
    delay(20);
  }
  // LPrintln("=== Buffer: ",sdiResponse); //write the response to the screen
  mySDI12.clearBuffer();
}


// reads just the next message in the Decagon buffer, stopping at CRLF
void read_next_message()
{
  sdiResponse = "";
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      break;  // stop at newline
    }
    sdiResponse += c;
    delay(20);
  }
  if (sdiResponse[sdiResponse.length()-1] == '\r') {
    sdiResponse[sdiResponse.length()-1] = '\0'; // replace carriage return with terminator
  }
  // LPrintln("=== Message: ",sdiResponse); //write the response to the screen
}

 
// broadcast query to SDI12 sensors, set sensor_address to the first response
void get_address()
{
  mySDI12.clearBuffer();
  
  myCommand = "?!";
  mySDI12.sendCommand(myCommand);
  // LPrintln("\n=== Sending command: ",myCommand);
  delay(200);
  read_buffer();
  sensor_address = sdiResponse[0];
  // LPrintln("address response: ", sdiResponse);
  // LPrintln("address as an int:", int(sensor_address));
  // LPrintln("address set to: ", sensor_address);
}


// sends a measure command and then a data request command
// Sets sdiResponse to the response message (which should contain the data)
void measure_decagon()
{
  // first command to take a measurement
  myCommand = String(sensor_address) + "M!";
  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  // the sensor will respond with some info which can be ignored here
  // the info includes the number of measurements to expect and time to wait for them
  // for future improvements, using this response could be useful
  read_buffer();
  mySDI12.clearBuffer();

  // next command to request data from last measurement
  myCommand = String(sensor_address) + "D0!";
  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  read_next_message();
  // if the sensor sent a service request before measurement data;
  // skip it and try for the next message, which should be the data
  if (sdiResponse.length() <= 3) {
    delay(30);
    read_next_message();
  }

  mySDI12.clearBuffer();
}


void package_decagon(){
    Feather.add_data("Decagon_GS3_M","moisture",dielec_p);
    Feather.add_data("Decagon_GS3_T","temperature",temp);
    Feather.add_data("Decagon_GS3_E","conductivity",elec_c);
}

void setup_decagon(){
  // initialize decagon sensor, get its address, send first command
  mySDI12.begin();
  delay(100);
  get_address();
  myCommand = String(sensor_address) + "I!";
  mySDI12.sendCommand(myCommand);
  delay(30);

  // read the decagon response, clear decagon buffer
  read_buffer();
  delay(30);
  mySDI12.clearBuffer();

  if(sensor_address != -1){
    Serial.print("\n=== Decagon initialized successfully\n");  
  } else {
    Serial.print("\n=== Decagon failed to initialize\n"); 
  }
}

void setup()
{
  while(!Serial);
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  Feather.begin_LED();
  Feather.begin_serial(true,true);
  setup_decagon();
  Feather.parse_config(json_config);
  Feather.print_config();

  getInterruptManager(Feather).register_ISR(12,ISR_pin12, LOW, ISR_Type::IMMEDIATE);

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

    pinMode(DATAPIN, OUTPUT);
  
    Feather.power_up();
  }
  
  Feather.measure();
  Feather.package();

  // get data from Decagon_GS3 sensor and package it
  // this must be done AFTER calls to Feather.package()
  // but before logging to SD
  measure_decagon();
  parse_results();
  package_decagon();

  Feather.display_data();
  getSD(Feather).log();
  //Feather.pause();
  
  getInterruptManager(Feather).RTC_alarm_duration(TimeSpan(0, 0, 0, 5));
  getInterruptManager(Feather).reconnect_interrupt(12);

  Feather.power_down();

  pinMode(23, INPUT);
  pinMode(24, INPUT);
  pinMode(10, INPUT);

  pinMode(DATAPIN, INPUT);

  // Protocol to turn off Hypnos
  digitalWrite(5, HIGH); // Disabling all pins before going to sleep.
  digitalWrite(6, LOW);

  getSleepManager(Feather).sleep();
  while (!flag); //waits for an interrupt flag
  
}

// Interrupt Functions
void ISR_pin12()
{
  detachInterrupt(12);
  flag = true;
}
