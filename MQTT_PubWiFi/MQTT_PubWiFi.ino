///////////////////////////////////////////////////////////////////////////////

// This is the simplest example of logging data to Google Sheets

// The only difference between this example an 'Basic' is the LoomFactory
// settings, the line:
//		Feather.GoogleSheets().publish();
// and the configuration, enabling logging to Google Sheets.

// In the config, you need:
// - WiFi network name and password (or '' if no password)
// - For Google sheets parameters, see:
//   https://github.com/OPEnSLab-OSU/Loom/wiki/Using-Loom-with-Google-Sheets

///////////////////////////////////////////////////////////////////////////////

#include <Loom.h>
#include "SDI12.h"
#include <String.h>

// Include configuration
const char* json_config =
#include "config.h"
;

// In Tools menu, set:
// Internet  > WiFi
// Sensors   > Enabled
// Radios    > Disabled
// Actuators > Enabled
// Max       > Disabled

using namespace Loom;

Loom::Manager Feather{};

#define DATAPIN 11         // change to the proper pin in which SDI12 is connected to on the Feather M0
SDI12 mySDI12(DATAPIN);

String sdiResponse = "";
String myCommand = "";
char   buf[20];
char   *p;
float  dielec_p = 0, temp = 0, elec_c = 0;
char sensor_address = 0;


// tokenize the sdiResponse into the three meaningful float data measurements
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


// reads the entire available sensor buffer (may read multiple messages)
void read_buffer()
{
  sdiResponse = "";
  delay(100);
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      sdiResponse += "<LF>";
    } else if (c == '\r') {
      sdiResponse += "<CR>";
    } else {
      sdiResponse += c;
    }
    delay(100);
  }
  // LPrintln("=== Buffer: ",sdiResponse); //write the response to the screen
  mySDI12.clearBuffer();
}


// reads just the next message in the buffer, stopping at CRLF
void read_next_message()
{
  sdiResponse = "";
  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if (c == '\n') {
      break;
    }
    sdiResponse += c;
    delay(100);
  }
  if (sdiResponse[sdiResponse.length()-1] == '\r') {
    sdiResponse[sdiResponse.length()-1] = '\0';
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
  delay(1000);
  read_buffer();
  sensor_address = sdiResponse[0];
  // LPrintln("address response: ", sdiResponse);
  // LPrintln("address as an int:", int(sensor_address));
  // LPrintln("address set to: ", sensor_address);
}


// sends a measure command and then a data request command
// Sets SDIResponse to the response message (which should contain the data)
void measure_decagon()
{
  // First command to take a measurement
  myCommand = String(sensor_address) + "M!";
  mySDI12.sendCommand(myCommand);
  delay(100);                     // wait a while for a response

  // the sensor will respond with some info which can be ignored here
  // the info includes the number of measurements to expect and time to wait
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

void setup()
{

  pinMode(5, OUTPUT);
  digitalWrite(5, LOW); // Sets pin 5, the pin with the 3.3V rail, to output and enables the rail
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); // Sets pin 6, the pin with the 5V rail, to output and enables the rail

  
  // initialize decagon sensor, get its address, send first command
  mySDI12.begin();
  delay(2000);
  get_address();
  myCommand = String(sensor_address) + "I!";
  mySDI12.sendCommand(myCommand);
  delay(30);

  // read the decagon response, clear decagon buffer
  read_buffer();
  delay(30);
  mySDI12.clearBuffer();
  
  
	Feather.begin_serial(true);
	Feather.parse_config(json_config);
	Feather.print_config();

	LPrintln("\n ** Setup Complete ** ");
}


void loop()
{
	Feather.measure();
	Feather.package();
  measure_decagon();
  parse_results();
  package_decagon();
  Feather.display_data();

	getMQTT_Pub(Feather).publish();

	Feather.pause();
}
