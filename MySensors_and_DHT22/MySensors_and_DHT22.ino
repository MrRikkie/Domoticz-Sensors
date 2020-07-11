/**
 ********************************
 *
 * REVISION HISTORY
 * Version 0.1: Erik Klessens
 * 
 * DESCRIPTION
 * This sketch uses NRF24 for sending measurements ( Temperature / Humidity ) to Mysensors network
 * Supported temperature sensors are : DHT11/DHT-22.
 * For more information how to connect the sensor and NRF24 wireless module see  :
 * http://www.mysensors.org/build/humidity
 * 
 ********************************
 * Libraries in use : 
 * Documentation: http://www.mysensors.org
 * DHTNew library : https://github.com/RobTillaart/DHTNew ( Version 0.1.7 )
 * 
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Erik Klessens
 * Copyright (C) 20020 Erik Klessens
 *
 * MySensors library : https://github.com/mysensors/MySensors
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 * 
 * DHTNew library : https://github.com/RobTillaart/DHTNew ( Version 0.1.7 )
 *
 */

//--------- SETUP Variables --------------------------------------------------------------------
// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached 
#define MY_RADIO_RF24
 
#include <SPI.h>
#include <MySensors.h>  
#include <dhtnew.h>

// Set this to the pin you connected the DHT's data pin to
#define DHT_DATA_PIN 3

// Set this offset if the sensor has a permanent small offset to the real temperatures.
// In Celsius degrees (as measured by the device)
#define SENSOR_TEMP_OFFSET 0.9
#define SENSOR_HUM_OFFSET 17

// Sleep time between sensor updates (in milliseconds)
// Must be >1000ms for DHT22 and >2000ms for DHT11
static const uint64_t UPDATE_INTERVAL = 60000;

// Force sending an update of the temperature after n sensor reads, so a controller showing the
// timestamp of the last update doesn't show something like 3 hours in the unlikely case, that
// the value didn't change since;
// i.e. the sensor would force sending an update every UPDATE_INTERVAL*FORCE_UPDATE_N_READS [ms]
static const uint8_t FORCE_UPDATE_N_READS = 10;

//--------- END SETUP Variables -----------------------------------------------------------------

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1

float lastTemp;
float lastHum;
uint8_t nNoUpdatesTemp;
uint8_t nNoUpdatesHum;
bool metric = true;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

DHTNEW mySensor(DHT_DATA_PIN);

void presentation()  
{ 
  // Send the sketch version information to the gateway
  sendSketchInfo("TemperatureAndHumidity", "0.1");
  
  // Register all sensors to gw (they will be created as child devices)
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  
  metric = getControllerConfig().isMetric;  // Can be used to convert values from Metric to Imperial ( not in use at this moment ) 
}


void setup()
{
  mySensor.setHumOffset(SENSOR_HUM_OFFSET);
  mySensor.setTempOffset(SENSOR_TEMP_OFFSET);
}


void loop()      
{  
  // Force reading sensor, so it works also after sleep()
  mySensor.read();
  
  // Get temperature from DHT library
  float temperature = mySensor.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT!");
  } else if (temperature != lastTemp || nNoUpdatesTemp == FORCE_UPDATE_N_READS) {
    // Only send temperature if it changed since the last measurement or if we didn't send an update for n times
    lastTemp = temperature;

    // Reset no updates counter
    nNoUpdatesTemp = 0;
    send(msgTemp.set(temperature, 1));

    #ifdef MY_DEBUG
    Serial.print("T: ");
    Serial.println(temperature);
    #endif
  } else {
    // Increase no update counter if the temperature stayed the same
    nNoUpdatesTemp++;
  }

  // Get humidity from DHT library
  float humidity = mySensor.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum || nNoUpdatesHum == FORCE_UPDATE_N_READS) {
    // Only send humidity if it changed since the last measurement or if we didn't send an update for n times
    lastHum = humidity;
    // Reset no updates counter
    nNoUpdatesHum = 0;
    send(msgHum.set(humidity, 1));
    
    #ifdef MY_DEBUG
    Serial.print("H: ");
    Serial.println(humidity);
    #endif
  } else {
    // Increase no update counter if the humidity stayed the same
    nNoUpdatesHum++;
  }

  // Sleep for a while to save energy
  sleep(UPDATE_INTERVAL); 
}
