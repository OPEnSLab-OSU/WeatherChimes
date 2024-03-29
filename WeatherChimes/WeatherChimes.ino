/**
 * In lab use case example for the WeatherChimes project
 * 
 * This project uses SDI12, TSL2591 and an SHT31 sensor to log environment data and logs it to both the SD card and also MQTT/MongoDB
 * 
 * MANAGER MUST BE INCLUDED FIRST IN ALL CODE
 */
#include <Loom_Manager.h>

#include <Hardware/Loom_Hypnos/Loom_Hypnos.h>

#include <Sensors/Loom_Analog/Loom_Analog.h>
#include <Sensors/I2C/Loom_SHT31/Loom_SHT31.h>
#include <Sensors/I2C/Loom_TSL2591/Loom_TSL2591.h>
#include <Sensors/I2C/Loom_MS5803/Loom_MS5803.h>
// If using Teros 10, Uncoment this line
#include <Sensors/Analog/Loom_Teros10/Loom_Teros10.h>

// If using SDI12, GS3 or Teros 11 or 12 uncoment this line
//#include <Sensors/SDI12/Loom_SDI12/Loom_SDI12.h>

#include <Internet/Logging/Loom_MQTT/Loom_MQTT.h>
#include <Internet/Connectivity/Loom_LTE/Loom_LTE.h>

Manager manager("4Gchime", 10);

// Create a new Hypnos object
Loom_Hypnos hypnos(manager, HYPNOS_VERSION::V3_3, TIME_ZONE::PST, true);

// Analog for reading battery voltage
Loom_Analog analog(manager);

// Create sensor classes
Loom_SHT31 sht(manager);
Loom_TSL2591 tsl(manager);
Loom_MS5803 ms_water(manager, 119); // 119(0x77) if CSB=LOW external, 118(0x76) if CSB=HIGH on WC PCB
Loom_MS5803 ms_air(manager, 118); // 118(0x76) if CSB=HIGH on WC PCB


Loom_LTE lte(manager, "hologram", "", "");
Loom_MQTT mqtt(manager, lte.getClient());

// If using Teros 10, Uncoment this line
Loom_Teros10 t10(manager, A0);

// If using SDI12, GS3 or Teros 11 or 12 uncoment this line
//Loom_SDI12 sdi(manager, A0);

// Called when the interrupt is triggered 
void isrTrigger(){
  hypnos.wakeup();
}

void setup() {

  // Wait 20 seconds for the serial console to open
  manager.beginSerial();

  // Enable the hypnos rails
  hypnos.enable();

  // Read the MQTT creds file to supply the device with MQTT credentials
  mqtt.loadConfigFromJSON(hypnos.readFile("mqtt_creds.json"));

  // Initialize all in-use modules
  manager.initialize();

  // Register the ISR and attach to the interrupt
  hypnos.registerInterrupt(isrTrigger);
}

void loop() {

  // Measure and package the data
  manager.measure();
  manager.package();
  
  // Print the current JSON packet
  manager.display_data();            

  // Log the data to the SD card              
  hypnos.logToSD();

  // Publish the collected data to MQTT
  mqtt.publish();
//  manager.pause(5000);

  // Set the RTC interrupt alarm to wake the device in 15 min
  hypnos.setInterruptDuration(TimeSpan(0, 0, 15, 0));

  // Reattach to the interrupt after we have set the alarm so we can have repeat triggers
  hypnos.reattachRTCInterrupt();
  
  // Put the device into a deep sleep, operation HALTS here until the interrupt is triggered
  hypnos.sleep();
  
}
