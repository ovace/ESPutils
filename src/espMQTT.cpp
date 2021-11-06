#include <Arduino.h>
#include <string>

#include "LittleFS.h" // LittleFS is declared
#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
#elif defined(ESP32)
  #include <WiFi.h> 
#elif
  #error Invalid platform
#endif 

#include <Ticker.h>
#include <AsyncMqttClient.h>

#include "espMQTT.h"
extern espMQTT myMqtt;
espMQTTcfg myMqttCfg;

MQTTconfig *mqttCfg, mymqttcfg;
// mqttCfg = &mymqttcfg;

static const char *cfgFilename = "/espMQTTconfig.json";  

#define _DEBUG_ mymqttcfg.debug
#define MQTT_RETRY_COUNT 10

// Raspberri Pi Mosquitto MQTT Broker
//#define MQTT_HOST IPAddress(192, 168, 1, XXX)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
// const char* MQTT_HOST = mymqttcfg.mqttcfg.server.c_str();
// const int MQTT_PORT = mymqttcfg.mqttcfg.port;
// #define MQTT_HOST = mymqttcfg.mqttcfg.server.c_str()
// #define MQTT_PORT = mymqttcfg.mqttcfg.port

//Initialize MQTT Client and Timer
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

// static long interval = 1000;        // Interval at which to publish sensor readings (1000=1sec Interval = 1 min)

static const char* HOST_NAME;
const char* MQTT_HOST;
int MQTT_PORT;

espMQTT::espMQTT() { //Class constructor
 
};
espMQTT::~espMQTT() { //Class destructor

};
void espMQTT::setup(){
  // read configuraton file
  myMqttCfg.loadConfiguration();
  
  if (_DEBUG_) {
    Serial.printf("Chip ID: %sS", String(ESP.getChipId()).c_str()); Serial.println();
    Serial.printf("Device Vcc: %s", String(ESP.getVcc()).c_str()); Serial.println();
    
    Serial.println("Configuration JSON File");
    myMqttCfg.printFile();

    Serial.println("Configuration: ");
    myMqttCfg.printCFG();
  }
  
  myMqtt.initMQTT();
  myMqtt.connectToMqtt();
};
void espMQTT::initMQTT() {  
  // HOST_NAME = strcpy("Sensor-", "Test"); //, mymqttcfg.devicecfg.location.c_str());
  HOST_NAME = "Sensor-Test"; //, mymqttcfg.devicecfg.location.c_str());
  MQTT_HOST = mymqttcfg.mqttcfg.server.c_str();
  MQTT_PORT = mymqttcfg.mqttcfg.port;

  mqttClient.onConnect(espMQTT::onMqttConnect);
  mqttClient.onDisconnect(espMQTT::onMqttDisconnect);
  mqttClient.onSubscribe(espMQTT::onMqttSubscribe);
  mqttClient.onUnsubscribe(espMQTT::onMqttUnsubscribe);
  mqttClient.onPublish(espMQTT::onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setClientId(HOST_NAME);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  if(_DEBUG_) {
    Serial.print("MQTT USER: "); Serial.println(mymqttcfg.mqttcfg.user.c_str());
    Serial.print("MQTT PSWD: "); Serial.println(mymqttcfg.mqttcfg.pas.c_str());
  }
 
  mqttClient.setCredentials(mymqttcfg.mqttcfg.user.c_str(), mymqttcfg.mqttcfg.pas.c_str());
};
void espMQTT::connectToMqtt() {  
  mqttClient.connect();  
  bool breakLoop = false;

while ( !mqttClient.connected())  {
    if (_DEBUG_){
      Serial.print("MQTT status: "); Serial.println(mqttClient.connected());
    };

    // mqtt not connected, reconnect here
    mqttClient.connect();
    int MQcount = 0;
    int UpCount = 0;
    Serial.println("Connecting to MQTT ");
    while (!mqttClient.connected() )
    {
      delay( 100 );
      Serial.print(".");
      if (UpCount >= 60)  // just keep terminal from scrolling sideways
      {
        UpCount = 0;
        Serial.println();
      }
      ++UpCount;
      ++MQcount;
      if (MQcount > MQTT_RETRY_COUNT) {
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
};
void espMQTT::onMqttConnect(bool sessionPresent) {
  Serial.println(sessionPresent);
}
void espMQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, espMQTT::setup);
  }
}
void espMQTT::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
 }
void espMQTT::onMqttUnsubscribe(uint16_t packetId) {
}
void espMQTT::onMqttPublish(uint16_t packetId) {
}
int espMQTT::mqttpub(const char* TOPIC, const char* PAYLOAD){
  // Publish an MQTT message on topic 
  if (_DEBUG_) {
    Serial.print("Topic: "); Serial.println(TOPIC);
    Serial.print("PAYLOAD: "); Serial.println(PAYLOAD);
    Serial.print("size of payload: "); Serial.println(sizeof(PAYLOAD));
  }
  uint16_t packetIdPub1 = mqttClient.publish(TOPIC, 1, true, PAYLOAD);
  return packetIdPub1;
  // return 1;
};
int espMQTT::mqttpub(const char* TOPIC, uint8_t qos = 1, bool retain = true, const char* PAYLOAD = (const char*)nullptr){

  if (_DEBUG_) {
    Serial.print("Topic: "); Serial.println(TOPIC);
    Serial.print("QOS: "); Serial.println(qos);
    Serial.print("Retained: "); Serial.println(retain);
    Serial.print("PAYLOAD: "); Serial.println(PAYLOAD);
    Serial.print("size of payload: "); Serial.println(sizeof(PAYLOAD));  
  }

  // Publish an MQTT message on topic 
  uint16_t packetIdPub1 = mqttClient.publish(TOPIC, qos , retain, PAYLOAD);

  return packetIdPub1;
};

espMQTTcfg::espMQTTcfg() { //Class constructor
 
};
espMQTTcfg::~espMQTTcfg() { //Class destructor

};
bool espMQTTcfg::loadConfiguration(const char *cfgFilename, MQTTconfig &mymqttcfg){
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
    Serial.print(F("Opening myMQTTcfg file: "));
    Serial.println(cfgFilename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(cfgFilename); 
      return 0;
      };

    const int capacity =   JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(5) +
                           150;
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
    mymqttcfg.cfg_rel = doc["release"] | 99.99;
    mymqttcfg.debug = doc["debug"] | false;

    //MQTT
    JsonObject MQTT = doc["MQTT"];
    mymqttcfg.mqttcfg.server =  MQTT["server"] | "";
    mymqttcfg.mqttcfg.port = MQTT["port"] | 1883;
    mymqttcfg.mqttcfg.user = MQTT["user"] | "";
    mymqttcfg.mqttcfg.pas = MQTT["pas"] | "";
    mymqttcfg.mqttcfg.home_topic_base = MQTT["home_topic_base"] | "esp/home";
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("File not found: ")); Serial.println(cfgFilename); 
    return 0;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return 1;
};
void espMQTTcfg::loadConfiguration(){
  myMqttCfg.loadConfiguration(cfgFilename, mymqttcfg);
};
void espMQTTcfg::saveConfiguration(const char *cfgFilename, const MQTTconfig &mymqttcfg) {
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
void espMQTTcfg::saveConfiguration(){
  myMqttCfg.saveConfiguration(cfgFilename, mymqttcfg);
};
void espMQTTcfg::printFile(const char *cfgFilename){
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
void espMQTTcfg::printFile(){
  myMqttCfg.printFile(cfgFilename);
};
void espMQTTcfg::printCFG(){
  Serial.printf("Release: %f ",mymqttcfg.cfg_rel); Serial.println();
  Serial.printf("Debug mode: %s ", mymqttcfg.debug? "true"  : "false");Serial.println();
  
  Serial.println(F("MQTT mymqttcfg"));
  Serial.println(mymqttcfg.mqttcfg.server);
  Serial.println(mymqttcfg.mqttcfg.user);
  Serial.println(mymqttcfg.mqttcfg.pas);
  Serial.println(mymqttcfg.mqttcfg.home_topic_base);
};
