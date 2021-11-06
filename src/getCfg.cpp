#include <Arduino.h>
#include <LittleFS.h> // LittleFS is declared
#include <ArduinoJson.h>
#include <string.h>

#include "getCfg.h"
espCFG myCfg;
GlobalConfig  config, *cfgP = &config;  // <- global configuration object -- cfg is the pointer 

const char *cfgFilename = "/mainConfig.json";  

#define _DEBUG_ cfgP->debug

espCFG::espCFG() { //Class constructor

};
espCFG::~espCFG() { //Class destructor

};
bool espCFG::setup(){  
  // read configuraton file
  myCfg.loadConfiguration();

  if (_DEBUG_) {
    Serial.printf("Chip ID: %s", String(ESP.getChipId()).c_str()); Serial.println();
    Serial.printf("Device Vcc: %s", String(ESP.getVcc()).c_str()); Serial.println();

    Serial.print("Configuration JSON File: ");Serial.println(cfgFilename);
    myCfg.printFile();

    Serial.println("Configuration: ");
    myCfg.printCFG();

  } 

  return true;
} 

// Loads the configuration from a file
GlobalConfig espCFG::loadConfiguration(const char *filename, GlobalConfig &config) {

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
  if (LittleFS.exists(filename)) {
    Serial.print(F("Opening config file")); ; Serial.println(filename); 
    File file = LittleFS.open(filename, "r");
    
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(filename); 
      return config;
      };

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const int capacity = 2*JSON_OBJECT_SIZE(1) + 
                         3*JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(4) +                            
                           JSON_OBJECT_SIZE(13) +
                           350;

    Serial.print("JsonBufferCapacity: "); Serial.println(capacity);
    StaticJsonDocument<capacity> doc;
    
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {  
      Serial.println(F("Failed to read file, using default configuration"));
      return config;
    };
    // Copy values from the JsonDocument to the Config

    //Release
    cfgP->cfg_rel = doc["release"] | 99.99;
    cfgP->debug = doc["debug"] | true;

    //Device
    JsonObject DEVICE = doc["Device"];
    cfgP->devicecfg.name = DEVICE["name"] | "";
    cfgP->devicecfg.type = DEVICE["type"] | "esp";
    cfgP->devicecfg.model = DEVICE["model"] | "esp8266";
    cfgP->devicecfg.location = DEVICE["location"] | "home";
    
    //Sensors
    JsonObject MODULES = doc["modules"];
    cfgP->moduleCfg.OTA =           MODULES["OTA"]           | true;
    cfgP->moduleCfg.MQTT =          MODULES["MQTT"]          | true;
    cfgP->moduleCfg.WebSvr =        MODULES["WebSvr"]        | false;
    cfgP->moduleCfg.Temperature =   MODULES["Temperature"]   | true;
    cfgP->moduleCfg.Humidity =      MODULES["Humidity"]      | true;
    cfgP->moduleCfg.Light =         MODULES["Light"]         | false;
    cfgP->moduleCfg.Soil_moisture = MODULES["Soil_moisture"] | false;
    cfgP->moduleCfg.Soil_salt =     MODULES["Soil_salt"]     | false;
    cfgP->moduleCfg.Battery =       MODULES["Battery"]       | true;
    cfgP->moduleCfg.Level =         MODULES["Level"]         | false;
    cfgP->moduleCfg.Water =         MODULES["Water"]         | false;
    cfgP->moduleCfg.ADC =           MODULES["ADC"]           | true;
    
    //Serial
    cfgP->serialcomcfg.espbaud = doc["Serial"]["espbaud"] |  115200; 

    //Timers
    JsonObject TIMERS = doc["Timers"];
    cfgP->timerscfg.MEASUREMENT_TIMEINTERVAL  = TIMERS["MEASUREMENT_TIMEINTERVAL"] |  6000; 
    cfgP->timerscfg.TIME_TO_SLEEP  = TIMERS["TIME_TO_SLEEP"] |  6000; 

    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("Failed not found: ")); Serial.println(filename); 
    return config;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return config;
}
GlobalConfig espCFG::loadConfiguration() {
  // GlobalConfig config;
  config = espCFG::loadConfiguration(cfgFilename, config);
  return config;
}
// Saves the configuration to a file
bool espCFG::saveConfiguration(const char *filename, const GlobalConfig *config) {

  //mounts file system.
  LittleFS.begin();
  // Delete existing file, otherwise the configuration is appended to the file
  LittleFS.remove(filename);

  // Open file for writing
  File file = LittleFS.open(filename, "r+");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return 0;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();

  return 1;
}
void espCFG::saveConfiguration() {
  // GlobalConfig config;
  espCFG::saveConfiguration(cfgFilename, cfgP);
}
// Prints the content of a file to the Serial
void espCFG::printFile(const char *filename) {

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
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.print(F("Failed to read file")); Serial.println(filename); 
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
}
void espCFG::printFile(){

  espCFG::printFile(cfgFilename);

}
void espCFG::printCFG() { 
  Serial.printf("Release: %f",cfgP ->cfg_rel);   Serial.println();
  Serial.printf("Debug mode: %s", cfgP ->debug? "true"  : "false");   Serial.println();

  Serial.println(F("Device Config")); 
  Serial.print(F("Device Name: "));Serial.println(cfgP ->devicecfg.name);
  Serial.print(F("Device Type: "));Serial.println(cfgP ->devicecfg.type);
  Serial.print(F("Device Model: "));Serial.println(cfgP ->devicecfg.model);
  Serial.print(F("Device Location: "));Serial.println(cfgP ->devicecfg.location);
 
  Serial.println(F("Module Config"));
  Serial.printf("Enable OTA: %s", cfgP ->moduleCfg.OTA?"Yes":"No"); Serial.println();
  Serial.printf("Enable OTA Pull: %s", cfgP ->moduleCfg.OTApull?"Yes":"No");    Serial.println(); 
  Serial.printf("MQTT: %s", cfgP ->moduleCfg.MQTT?"Yes":"No");   Serial.println();
  Serial.printf("WebSvr: %s", cfgP ->moduleCfg.WebSvr?"Yes":"No");   Serial.println();
  Serial.printf("Temperature: %s", cfgP ->moduleCfg.Temperature?"Yes":"No");   Serial.println();
  Serial.printf("Humidity: %s", cfgP ->moduleCfg.Humidity?"Yes":"No");   Serial.println();
  Serial.printf("Light: %s", cfgP ->moduleCfg.Light?"Yes":"No");   Serial.println();
  Serial.printf("Soil_Moisture: %s", cfgP ->moduleCfg.Soil_moisture?"Yes":"No");   Serial.println();
  Serial.printf("Soil_salt: %s", cfgP ->moduleCfg.Soil_salt?"Yes":"No");  Serial.println();
  Serial.printf("Battery: %s", cfgP ->moduleCfg.Battery?"Yes":"No");   Serial.println();
  Serial.printf("Level: %s", cfgP ->moduleCfg.Level?"Yes":"No");   Serial.println();
  Serial.printf("Water: %s", cfgP ->moduleCfg.Water?"Yes":"No");   Serial.println();
  Serial.printf("ADC: %s", cfgP ->moduleCfg.ADC?"Yes":"No");   Serial.println();

  // Serial.println(F("NTP Config"));
  // Serial.println(cfgP ->ntpcfg.Server);
  // Serial.println(cfgP ->ntpcfg.refrehFreq);

  Serial.println(F("WebServer Config"));
  Serial.print(F("WebSvr Port: "));Serial.println(cfgP ->websvrcfg.port);

  Serial.println(F("Serial Config"));
  Serial.print(F("Serial baud: "));Serial.println(cfgP ->serialcomcfg.espbaud);

  Serial.println(F("Timers Config"));
  Serial.print(F("Timeers Interval: "));Serial.println(cfgP ->timerscfg.MEASUREMENT_TIMEINTERVAL);
  Serial.print(F("Timers Sleep Time: "));Serial.println(cfgP ->timerscfg.TIME_TO_SLEEP);  
}