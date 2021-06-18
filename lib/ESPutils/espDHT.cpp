/*
  @file espDHT.cpp


*/
#include <Arduino.h>
#include <string>

#include <DHT.h>

#include "config.h"
#include "getCfg.h"
#include "espDHT.h"

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);


static long interval = 1000;        // Interval at which to publish sensor readings (1000=1sec Interval = 1 min)

espDHT::espDHT() { //Class constructor
};
espDHT::~espDHT() { //Class destructor
};

void espDHT::DHTsetup(){
  //Serial.printf("DHTTYPE: %s", DHTTYPE);  
  espDHT::initDHT();
};

void espDHT::initDHT() {
  dht.begin(); // Connect DHT sensor to GPIO 2  
}

float espDHT::readDHTTemp(bool isFarenheit){
// Variables to hold sensor readings
  float temperature;

 // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();
  
  // The sensor is reading about 10% high, hence this correction
  ////temperature = temperature * 90/100;
  
  //temp = dht.readTemperature(true);
  //temperature = dht.getTemperature(true);
  if (isFarenheit)
    {
      temperature = dht.convertCtoF(temperature) * 90/100;
    }
  return temperature;
}

float espDHT::readDHTHumid(){

  // Variables to hold sensor readings
  float humidity;


  // New DHT sensor readings
  //hum = dht.readHumidity();
  humidity = dht.readHumidity();

  return humidity;
};


sensorReady espDHT::sensorlatency(unsigned long previousMillis, int dly){
  // minimum wait between sensor reads
  interval = espDHT::getMinimumSamplingPeriod();

  sensorReady val = {0, previousMillis};
  
   unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval + dly) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

    val.ready= 1;
    val.previosMillis = previousMillis;

    return val;

  }
  return val;
};

