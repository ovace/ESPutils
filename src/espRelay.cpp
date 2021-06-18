/*
  @file espRelay.cpp


*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

// Required for MQTT
#if defined(ESP8266)  
  #include <ESP8266WebServer.h>  
#elif defined(ESP32)  
  #include <WebServer.h>
#else
  #error Invalid platform
#endif 

#include "config.h"
#include "getCfg.h"
#include "espRelay.h"
#include "espMQTT-WiFi.h"
#include "espTime.h"
// #include "SwitchData.h"  // inclued in header file

static espMQTT mymqtt;
static espTime mytime;
static espSend mysend;
static SwitchData switchdata;

int sendCount = 0;

espRelay::espRelay() { //Class constructor
  mycfg.loadConfiguration();
};
espRelay::~espRelay() { //Class destructor
};
espSend::espSend() { //Class constructor
};
espSend::~espSend() { //Class destructor
};

void espRelay::setupRelay() {  
  mycfg.loadConfiguration();
};

bool espRelay::RelayStatus() {
  return true;
};

bool espRelay::relayClose() {
  Serial.write("\xa0\x01\x01\xa2"); // CLOSE RELAY 
  switchdata.SW_State = 1;

  mysend.sendMsg();

  return 1;
}

bool espRelay::relayOpen() {
  Serial.write("\xa0\x01"); // OPEN RELAY
  Serial.write(0x00); // null terminates a string so it has to be sent on its own
  Serial.write(0xa1);

  switchdata.SW_State = 0;

  mysend.sendMsg();

  return 0;
};

// Send message to MQTT
bool espSend::sendMsg() { 

// Send to mqtt
  const String topicStr = String(config.mqttcfg.home_topic_base) + "/" + config.devicecfg.location + "/" + config.devicecfg.name;
  const char* topic = topicStr.c_str();

  Serial.println(F("Sending to MQTT, on topic"));
  Serial.println(topic);
  sendCount++;
  switchdata.sendCount = sendCount;
  mysend.prepData();   
  auto payload = mysend.createJSON(switchdata);  
  mysend.sendMQTT(topic, payload);    
   
  return 1;
};

//get date and Publich to MQTT
SwitchData espSend::prepData() {

  switchdata.SW_Name = config.devicecfg.name + "-" +  config.devicecfg.location;
    
    // Extract date dd-mm
    switchdata.date = mytime.getCurDate();
    // Extract time HH:MM    
    switchdata.time = mytime.getCurTimestamp();
    
    //Total device uptime since last reset
    // switchdata.upTime = difftime(time(nullptr), progstats.startTime);

    return switchdata;
  
}

// Allocate a  JsonDocument
StaticJsonDocument<1024> espSend::createJSON(SwitchData & switchdata) {
  StaticJsonDocument<1024> doc;
  // Set the values in the document
  // Device changes according to device placement
  JsonObject Switch = doc.createNestedObject(switchdata.SW_Name);
    Switch["Location"] = config.devicecfg.location;
    Switch["Name"] = config.devicecfg.name;
    Switch["SWname"] = switchdata.SW_Name;
    Switch["State"] = switchdata.SW_State;
  JsonObject times = doc.createNestedObject("times");
    times["date"] = switchdata.date;
    times["time"] = switchdata.time;
    times["upTime"] = switchdata.upTime;
    times["send_Interval"] = switchdata.interval;
  JsonObject Counters = doc.createNestedObject("Counters");
    Counters["bootCount"] = switchdata.bootno;  
    Counters["sendCount"] = switchdata.sendCount;  
  JsonObject rel = doc.createNestedObject("rel");
    rel["code_rel"] = switchdata.code_rel;
    rel["conf_rel"] = switchdata.cfg_rel;
  JsonObject Power = doc.createNestedObject("Power");
    Power["Vcc"] = switchdata.Vcc;

  return doc;
};

void espSend::sendMQTT(const char* topic, StaticJsonDocument<1024>& JsonDoc){
  // Create a buffer
  char buffer[1024];
  serializeJson(JsonDoc, buffer);

  Serial.printf("Sending message to topic: %s", topic);
  
  Serial.println(buffer);

  // bool retained = true;
  // int qos = 0;

  // Publish an MQTT message on topic esp/dht/temperature
  mymqtt.mqttpub(topic, buffer);
  
  Serial.println();

};

void espSend::sendMQTT(const char* topic, String payload){ 

  // bool retained = true;
  // int qos = 0;

  // Publish an MQTT message on topic esp/dht/temperature
  mymqtt.mqttpub(topic, payload.c_str());
  
};

