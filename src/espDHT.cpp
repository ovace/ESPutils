/*
  @file espDHT.cpp


*/
#include <Arduino.h>
#include <string>

#include "LittleFS.h" // LittleFS is declared
#include <ArduinoJson.h>

#include <Ticker.h>
#include <DHT.h>

#include "espDHT.h"
extern espDHT myDht;
espDHTcfg myDhtCfg;

DHTconfig mydhtcfg, *dhtCfg = &mydhtcfg;

static const char *cfgFilename = "/espDHTconfig.json";  

#define _DEBUG_ mydhtcfg.debug
#define DHTPIN mydhtcfg.dhtcfg.pin
#define DHTTYPE mydhtcfg.dhtcfg.type

// long interval = 500;        // Interval at which to publish sensor readings (1000=1sec Interval = 1 min)

// Initialize DHT sensor
  DHT dht(4, 11);


espDHT::espDHT() { //Class constructor
};
espDHT::~espDHT() { //Class destructor
};
void espDHT::setup(){
  // read configuraton file
  myDhtCfg.loadConfiguration();

  Serial.printf("Debug: %s", _DEBUG_?"Yes":"No"); Serial.println();

  if (_DEBUG_){
    
    Serial.println("Configuration JSON File");
    myDhtCfg.printFile();

    Serial.println("Configuration: ");
    myDhtCfg.printCFG();
  }
  //Serial.printf("DHTTYPE: %s", DHTTYPE);  
  espDHT::initDHT();
};
void espDHT::initDHT() {
  // Initialize DHT sensor
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin(); // Connect DHT sensor to GPIO 4
}
float espDHT::readTemp(bool isFarenheit){
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
float espDHT::readHumid(){

  // Variables to hold sensor readings
  float humidity;


  // New DHT sensor readings
  //hum = dht.readHumidity();
  humidity = dht.readHumidity();

  return humidity;
};
sensorReady espDHT::sensorlatency(unsigned long previousMillis, int dly){
  // minimum wait between sensor reads
  long interval = espDHT::getMinimumSamplingPeriod();

  sensorReady val = {0, previousMillis};

  unsigned long currentMillis = millis();

   if (_DEBUG_) {
      Serial.printf("Mills befor delay: %lu", currentMillis); Serial.println();
   }

  delay(interval);
  
  currentMillis = millis();

  if (_DEBUG_) {
    Serial.printf("interval: %li", interval); Serial.println();
    // Serial.printf("val: %d", val.ready); Serial.println();
    Serial.printf("currentMills: %lu", currentMillis); Serial.println();
    Serial.printf("previousMills: %lu", previousMillis); Serial.println();
  }
    
  if (currentMillis - previousMillis >= (unsigned long)(interval + dly)) {
    // Save the last time a new reading was published
    previousMillis = currentMillis;

    val.ready= 1;
    val.previosMillis = previousMillis;

    if (_DEBUG_) {
    Serial.printf("interval: %li", interval); Serial.println();
    // Serial.printf("val: %d", val.ready); Serial.println();
    Serial.printf("currentMills: %lu", currentMillis); Serial.println();
    Serial.printf("previousMills: %lu", previousMillis); Serial.println();
    }

    return val;

  }
  return val;
};
int espDHT::getMinimumSamplingPeriod() { 
  return (mydhtcfg.dhtcfg.type != DHT11 ? 1000 : 2000); 
};

espDHTcfg::espDHTcfg() { //Class constructor
 
};
espDHTcfg::~espDHTcfg() { //Class destructor

};
void espDHTcfg::loadConfiguration(const char *cfgFilename, DHTconfig &mydhtcfg){
//mounts file system.
  LittleFS.begin();

  FSInfo fs_info;
  LittleFS.info(fs_info);
    Serial.print(F("Total Bytes: "));Serial.println( fs_info.totalBytes);
    Serial.print(F("used Bytes: "));Serial.println(fs_info.usedBytes);
    Serial.print(F("Block Size: "));Serial.println(fs_info.blockSize);
    Serial.print(F("Page Size: "));Serial.println(fs_info.pageSize);
    Serial.print(F("max open files: "));Serial.println(fs_info.maxOpenFiles);
    Serial.print(F("max path lengt: "));Serial.println(fs_info.maxPathLength);

  // Open file for reading
  if (LittleFS.exists(cfgFilename)) {

    File file = LittleFS.open(cfgFilename, "r");
    Serial.print(F("Opening mydhtcfg file: "));
    Serial.println(cfgFilename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(cfgFilename); 
      return;
      };

    const int capacity =   2*JSON_OBJECT_SIZE(2) + 
                           100;
    Serial.print("JsonBufferCapacity: "); Serial.println(capacity);
    StaticJsonDocument<capacity> doc;

    // StaticJsonDocument<300> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {  
      Serial.print(F("Failed to read file, using default configuration"));
      Serial.println(error.f_str());
      return;
    };
    // Copy values from the JsonDocument to the myadccfg

    //Release
    mydhtcfg.cfg_rel = doc["release"] | 99.99;
    mydhtcfg.debug = doc["debug"] | false;

    //DHT
    JsonObject DHT = doc["DHT"];
    mydhtcfg.dhtcfg.pin  = DHT["pin"] |  4; 
    mydhtcfg.dhtcfg.type  = DHT["type"] |  11; 
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("Failed not found: ")); Serial.println(cfgFilename); 
    return;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return;
};
void espDHTcfg::loadConfiguration(){
  myDhtCfg.loadConfiguration(cfgFilename, mydhtcfg);
};
void espDHTcfg::saveConfiguration(const char *cfgFilename, const DHTconfig &mydhtcfg) {
  //mounts file system.
  LittleFS.begin();
  // Delete existing file, otherwise the configuration is appended to the file
  LittleFS.remove(cfgFilename);

  // Open file for writing
  File file = LittleFS.open(cfgFilename, "r+");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  // doc["hostname"] = myadccfg.wirelesscfg.hostname;
  // doc["port"] = myadccfg.wirelesscfg.port;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();
};
void espDHTcfg::saveConfiguration(){
  myDhtCfg.saveConfiguration(cfgFilename, mydhtcfg);
};
void espDHTcfg::printFile(const char *cfgFilename){
  //mounts file system.
  LittleFS.begin();

  FSInfo fs_info;
  LittleFS.info(fs_info);

  Serial.println( fs_info.totalBytes);
  Serial.println(fs_info.usedBytes);
  Serial.println(fs_info.blockSize);
  Serial.println(fs_info.pageSize);
  Serial.println(fs_info.maxOpenFiles);
  Serial.println(fs_info.maxPathLength);

  // Open file for reading
  File file = LittleFS.open(cfgFilename, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();
};
void espDHTcfg::printFile(){
  myDhtCfg.printFile(cfgFilename);
};
void espDHTcfg::printCFG(){
  Serial.printf("Release: %f",mydhtcfg.cfg_rel); Serial.println();
  Serial.printf("Debug mode: %s", mydhtcfg.debug? "true"  : "false"); Serial.println();
  
  Serial.println(F("DHT Config"));
  Serial.println(mydhtcfg.dhtcfg.pin);
  Serial.println(mydhtcfg.dhtcfg.type);
};
