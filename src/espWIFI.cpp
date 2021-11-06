/*
  @file connect-to-network.cpp


*/
#include <Arduino.h>
#include <string>

#include "LittleFS.h" // LittleFS is declared
#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
  // #include <WiFiClient.h>
  #include <ESP8266mDNS.h>  
#elif defined(ESP32)
  #include <WiFi.h>   
  #include <mdns.h>
  #include "SPIFFS.h"   
#else
  #error Invalid platform
#endif 

#include <include/WiFiState.h> // WiFiState structure details
WiFiState state; // Size of this is 152 bytes

#ifndef RTC_USER_DATA_SLOT_WIFI_STATE
#define RTC_USER_DATA_SLOT_WIFI_STATE 33u // Data stored in first 32 blocks overwritten by OTA updates
#endif

#include "espWIFI.h"
extern espNWconn myNwCon;
espNWcfg myNwCfg;
#include "go-to-deep-sleep.h"
extern go2Sleep go2sleep;

NWconfig *nwCfg, mynwcfg;
// mqttCfg = &mymqttcfg;

static const char *cfgFilename = "/espWIFIconfig.json";  

#define _DEBUG_ mynwcfg.debug
#define HOST_NAME mynwcfg.wirelesscfg.hostname 
// #define WIFI_STA mynwcfg.wirelesscfg.espmode
#define SSID  mynwcfg.wirelesscfg.ssid
#define PSK mynwcfg.wirelesscfg.psk
#define WIFI_RETRY_COUNT mynwcfg.wirelesscfg.connRetries

espNWconn::espNWconn() { //Class constructor
  // HOST_NAME = "Sensor-"+ mynwcfg.devicecfg.location;
  
};
espNWconn::~espNWconn() { //Class destructor
};
void espNWconn::setup(){
  // read WiFistate from RTC user Memory
  ESP.rtcUserMemoryRead(RTC_USER_DATA_SLOT_WIFI_STATE, reinterpret_cast<uint32_t *>(&state), sizeof(state));

  // read configuraton file
  myNwCfg.loadConfiguration();
   
  if (_DEBUG_) {
    Serial.print("SSID "); Serial.println(SSID);
    Serial.print("PSK ");  Serial.println(PSK); 

    Serial.println(F("WiFi State info"));
    Serial.print("CRC: ");Serial.println(state.crc);
    Serial.print("Channel: ");Serial.println(state.state.channel);
    Serial.print("DNS: ");Serial.println(state.state.dns->addr);
    Serial.print("ip: ");Serial.println(state.state.ip.ip.addr);
    Serial.print("mode: ");Serial.println(state.state.mode);
    Serial.print("ntp: ");Serial.println(state.state.ntp->addr);
    Serial.println();

    Serial.println("Configuration JSON File");
    myNwCfg.printFile();

    Serial.println("Configuration: ");
    myNwCfg.printCFG();
  }
  // HOST_NAME = mynwcfg.devicecfg.location+ "-"+mynwcfg.wirelesscfg.hostname+ "-"+mynwcfg.devicecfg.place;
  // HOST_NAME = "Sensor-"+ mynwcfg.devicecfg.location;

  Serial.println("Hostname: " + HOST_NAME);

  WiFi.mode(WIFI_STA);
  espNWconn::connect();
  
}; 
void espNWconn::getCfg(){
  
};
void espNWconn::connect() {  
  WiFi.begin(SSID, PSK);
  delay(5);
  WiFi.hostname(HOST_NAME);
  
  bool breakLoop = false;
  // if (logging) {
  //   espfilemgmt.writeFile(SPIFFS, "/error.log", "Connecting to Network: \n");
  // }
  /* 
  for (int i = 0; i < ssidArrNo; i++) {
    ssid = ssidArr[i].c_str();
    Serial.print("SSID name: ");
    Serial.print(ssidArr[i]);
  */
  
  // while ( WiFi.status() !=  WL_CONNECTED )
  while ( WiFi.waitForConnectResult() !=  WL_CONNECTED )
  {
    if (_DEBUG_){
      Serial.print("WiFi status: "); Serial.println(WiFi.waitForConnectResult());
    };

    // wifi down, reconnect here
    WiFi.begin(SSID, PSK);
    int WLcount = 0;
    int UpCount = 0;
    Serial.print("Connecting to WiFi ");
    while (WiFi.waitForConnectResult() != WL_CONNECTED )
    {
      delay( 100 );
      Serial.print(".");
      if (UpCount >= 60)  // just keep terminal from scrolling sideways
      {
        UpCount = 0;
        Serial.println();
      }
      ++UpCount;
      ++WLcount;
      if (WLcount > WIFI_RETRY_COUNT) {
        Serial.println("retries exceede -- Break");
        breakLoop = true;
        break;
      }
    }
    if (breakLoop) {
      breakLoop = false;
      break;
    }
  }
  
  if (WiFi.waitForConnectResult() !=  WL_CONNECTED) {
    go2sleep.goToDeepSleep(30);
    myNwCon.WiFiOn();
  }
  #ifdef ESP32
    mdns_hostname_set(HOST_NAME.c_str());
  #endif

  //write WiFi state information to RTC User Memory
  ESP.rtcUserMemoryWrite(RTC_USER_DATA_SLOT_WIFI_STATE, reinterpret_cast<uint32_t *>(&state), sizeof(state));
}
void espNWconn::WiFiOff() {
  WiFi.shutdown(state);
};
void espNWconn::WiFiOn() {
  WiFi.resumeFromShutdown(state);
};
void espNWconn::WiFiSleep() {
  WiFi.forceSleepBegin();
};
void espNWconn::WiFiAwake() {
  WiFi.forceSleepWake();
};

espNWcfg::espNWcfg() { //Class constructor
 
};
espNWcfg::~espNWcfg() { //Class destructor

};
bool espNWcfg::loadConfiguration(const char *cfgFilename, NWconfig &mynwcfg){
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
    Serial.print(F("Opening mywificfg file: "));
    Serial.println(cfgFilename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(cfgFilename); 

      return 0;
      };

    const int capacity =   JSON_OBJECT_SIZE(1) + 
                           JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(4) +                            
                           JSON_OBJECT_SIZE(6) +
                           300;
    Serial.print("JsonBufferCapacity: "); Serial.println(capacity);
    StaticJsonDocument<capacity> doc;
    // StaticJsonDocument<300> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {  
      Serial.print(F("Failed to read file, using default configuration"));
      Serial.println(error.f_str());
      return 0;
    };
    // Copy values from the JsonDocument to the myadccfg

    //Release
    mynwcfg.cfg_rel = doc["release"] | 99.99;
    mynwcfg.debug = doc["debug"] | false;

    //WiFi
    JsonObject DEVICE = doc["Device"];    
    mynwcfg.wirelesscfg.hostname = DEVICE["name"] | "esp";

    JsonObject WIFI = doc["WiFi"];
    mynwcfg.wirelesscfg.port = WIFI["port"] | 2731;
    mynwcfg.wirelesscfg.ssid = WIFI["ssid"] | "";
    // strlcpy(mynwcfg.wirelesscfg.psk,  WIFI["psk"] | "", sizeof(mynwcfg.wirelesscfg.psk));
    mynwcfg.wirelesscfg.psk = WIFI["psk"] | "";
    mynwcfg.wirelesscfg.espmode = WIFI["espmode"] | "WIFI_STA";
    mynwcfg.wirelesscfg.connRetries = WIFI["connRetries"] |10; 
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("File not found: ")); Serial.println(cfgFilename); 
    return 0;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return 1;
};
void espNWcfg::loadConfiguration(){
  myNwCfg.loadConfiguration(cfgFilename, mynwcfg);
};
void espNWcfg::saveConfiguration(const char *cfgFilename, const NWconfig &mynwcfg) {
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
void espNWcfg::saveConfiguration(){
  myNwCfg.saveConfiguration(cfgFilename, mynwcfg);
};
void espNWcfg::printFile(const char *cfgFilename){
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
void espNWcfg::printFile(){
  myNwCfg.printFile(cfgFilename);
};
void espNWcfg::printCFG(){
  Serial.printf("Release: %f",mynwcfg.cfg_rel); Serial.println();
  Serial.printf("Debug mode: %s", mynwcfg.debug? "true"  : "false"); Serial.println();
  
  Serial.println(F("WiFi Config"));
  Serial.println(mynwcfg.wirelesscfg.hostname);
  Serial.println(mynwcfg.wirelesscfg.port);
  Serial.println(mynwcfg.wirelesscfg.ssid);
  Serial.println(mynwcfg.wirelesscfg.psk);
  Serial.println(mynwcfg.wirelesscfg.espmode);
  Serial.println(mynwcfg.wirelesscfg.connRetries);
};
