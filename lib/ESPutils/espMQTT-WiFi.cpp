#include <Arduino.h>
#include <string>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
  #include <WiFiClient.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h> 
#else
  #error Invalid platform
#endif 

#include <Ticker.h>
#include <AsyncMqttClient.h>

#include "config.h"
#include "getCfg.h"
#include "espMQTT-WiFi.h"

wifiStat WS;

const char* ssid = config.wirelesscfg.ssid.c_str();
// const char* psk = config.wirelesscfg.psk.c_str();
const char* psk = config.wirelesscfg.psk;
const char* host_name = config.wirelesscfg.hostname.c_str();

// Raspberri Pi Mosquitto MQTT Broker
//#define MQTT_HOST IPAddress(192, 168, 1, XXX)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
const char* MQTT_HOST = config.mqttcfg.server.c_str();
const int MQTT_PORT = config.mqttcfg.port;

//Initialize MQTT Client and Timer
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

// Initialize WiFiEventHandler and Timer
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

long interval = 1000;        // Interval at which to publish sensor readings (1000=1sec Interval = 1 min)

espMQTT::espMQTT() { //Class constructor

};
espMQTT::~espMQTT() { //Class destructor

};

espWiFi::espWiFi() { //Class constructor

};
espWiFi::~espWiFi() { //Class desstructor

};

espSensor::espSensor() { //Class constructor
};

espSensor::~espSensor() { //Class destructor
};


void espWiFi::wifiSetup() {  
  espWiFi::connectToWifi();  
  espWiFi::initWifi();
};

void espWiFi::connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  int conCounter = 0;
  WiFi.mode(esp_mode);  
  WiFi.hostname(host_name); 
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, psk);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);    
    conCounter++;
    // if not connected in certain time, reset and retry
    if (conCounter > conRetry) {
      ESP.restart();
    };
  };  

  if (MDNS.begin(host_name)) {    
    delay(500);

    MDNS.update();
  }

  
  delay(500);
  espWiFi::wifiStatus(); 
};

void espWiFi::initWifi() {
  wifiConnectHandler = WiFi.onStationModeGotIP(espWiFi::onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(espWiFi::onWifiDisconnect);
};

void espWiFi::onWifiConnect(const WiFiEventStationModeGotIP& event) {
  espMQTT::mqttSetup();
}

void espWiFi::onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, espWiFi::connectToWifi);
};

wifiStat espWiFi::wifiStatus() {
  
  // Check for WiFi connection
 if (WiFi.status() == WL_CONNECTED) {   
     WS.connState = 1;
    } 
    else {
      WS.connState = 0;
  };

  WS.mac = WiFi.macAddress().c_str(); 
  WS.ssid = WiFi.SSID().c_str();  
  WS.ip = WiFi.localIP();  
  WS.hostname = WiFi.hostname().c_str();
  WS.rssi = WiFi.RSSI();  
   
  return WS;
};


void espMQTT::mqttSetup(){
  espMQTT::initMQTT();
  espMQTT::connectToMqtt();
};

void espMQTT::initMQTT() {  
  mqttClient.onConnect(espMQTT::onMqttConnect);
  mqttClient.onDisconnect(espMQTT::onMqttDisconnect);
  mqttClient.onSubscribe(espMQTT::onMqttSubscribe);
  mqttClient.onUnsubscribe(espMQTT::onMqttUnsubscribe);
  mqttClient.onPublish(espMQTT::onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setClientId(host_name);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  mqttClient.setCredentials(config.mqttcfg.user.c_str(), config.mqttcfg.pas.c_str());
};

void espMQTT::connectToMqtt() {  
  mqttClient.connect();
};

void espMQTT::onMqttConnect(bool sessionPresent) {
  Serial.println(sessionPresent);
}

void espMQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, espMQTT::mqttSetup);
  }
}

void espMQTT::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
 }

void espMQTT::onMqttUnsubscribe(uint16_t packetId) {
}

void espMQTT::onMqttPublish(uint16_t packetId) {
}

void espMQTT::mqttpub(const char* TOPIC, const char* PAYLOAD){
  // Publish an MQTT message on topic 
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC, 1, true, PAYLOAD);
};


sensorReady espSensor::sensorlatency(unsigned long previousMillis, int dly){
  // minimum wait between sensor reads
  interval = TIME_TO_WAIT;

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

