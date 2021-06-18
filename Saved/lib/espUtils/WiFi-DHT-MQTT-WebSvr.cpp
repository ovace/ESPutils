/*
  @file WiFi-DHT-MQTT-WebSvr.cpp


*/
#include <Arduino.h>
#include <string>

/* 
#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif 
*/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
  #include <ESP8266WebServer.h>
  #include <WiFiClient.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
#else
  #error Invalid platform
#endif 

#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <DHT.h>

#include "index.html"  //include the index.html page
#include "config.h"
#include "getCfg.h"
#include "WiFi-DHT-MQTT-WebSvr.h"

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

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

//Initialize Webserver
ESP8266WebServer server(wsport);

//Initialize MQTT Client and Timer
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

// Initialize WiFiEventHandler and Timer
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

long interval = 1000;        // Interval at which to publish sensor readings (1000=1sec Interval = 1 min)

espWiFi::espWiFi() { //Class constructor
};

espDHT::espDHT() { //Class constructor
};

espMQTT::espMQTT() { //Class constructor
};

espWebSrv::espWebSrv() { //Class constructor
};


void espWiFi::wifiSetup() {    
  if ( _DEBUG_ ) {
    Serial.println(F("WiFi connect Debug info"));
    Serial.printf("SSID: %s \n", ssid);
    Serial.printf("Pass: %s \n", psk);
    Serial.printf("Hostname: %s \n", host_name);
    Serial.printf("Connection retry:  %d  times:\n", conRetry);
    mycfg.printCFG();
  };
  espWiFi::connectToWifi();  
  espWiFi::initWifi();
};

void espWiFi::connectToWifi() {  
  Serial.printf("Connecting to ssid %s and will retry for %d times:\n", ssid, conRetry);
  int conCounter = 0;
  WiFi.mode(esp_mode);
  //wifi_station_set_hostname(ESP_HOSTNAME);
  WiFi.hostname(host_name);
  //DbgPrintln("hostname::", WiFi.hostname());
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, psk);
  
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(interval);
    Serial.print(".");
    conCounter++;
    // if not connected in certain time, reset and retry
    if (conCounter > conRetry) {
      Serial.println(F("unable to connect to AP - resetting to attempt again"));
      ESP.restart();
    };
  };  

  if (MDNS.begin(host_name)) {
    Serial.println(F("MDNS responder started"));
    delay(100);

    MDNS.update();
  }

  // WS.mac = WiFi.macAddress().c_str();
  // WS.ssid = WiFi.SSID().c_str();
  // WS.ip = WiFi.localIP();
  // WS.hostname = WiFi.hostname().c_str();
  // //WS.status = WiFi.status();
  // WS.rssi = WiFi.RSSI();

  delay(100);

  if ( _DEBUG_) {
    Serial.println(F("WiFi status Debug info"));
    espWiFi::wifiStatus();
  };
};
  

void espWiFi::initWifi() {
  wifiConnectHandler = WiFi.onStationModeGotIP(espWiFi::onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(espWiFi::onWifiDisconnect);
};

void espWiFi::onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  espMQTT::mqttSetup();
}

void espWiFi::onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
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

  Serial.println("");
  Serial.printf("Connected: %d \n", WS.connState); 
  WS.mac = WiFi.macAddress().c_str();
  Serial.printf("mac address:%X.%X.%X.%X \n", WS.mac[0], WS.mac[1], WS.mac[2], WS.mac[3]);  
  WS.ssid = WiFi.SSID().c_str();
  Serial.printf("Connected to: %s \n", WS.ssid);  
  WS.ip = WiFi.localIP();
  //IPAddress ip = WiFi.localIP();
  Serial.printf("IP Address: %u.%u.%u.%u \n", WS.ip[0], WS.ip[1], WS.ip[2], WS.ip[3]);
  WS.hostname = WiFi.hostname().c_str();
  Serial.printf("Host Name: %s \n", WS.hostname);  
  // WS.status = WiFi.status();
  // Serial.printf("Connection status: %s\n", WiFi.status() );
  WS.rssi = WiFi.RSSI();
  Serial.printf("RSSI: %ld dBm \n", WS.rssi);  
   
  return WS;
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
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
};

void espMQTT::onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void espMQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, espMQTT::mqttSetup);
  }
}

void espMQTT::onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void espMQTT::onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void espMQTT::onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void espMQTT::mqttpub(const char* TOPIC, const char* PAYLOAD){
  // Publish an MQTT message on topic 
    uint16_t packetIdPub1 = mqttClient.publish(TOPIC, 1, true, PAYLOAD);                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", TOPIC, packetIdPub1);
    Serial.printf("Message: %s \n", PAYLOAD);
};


void espWebSrv::setupWebSvr(const int port) {
  // espWebSrv::initWebSvr(port);
  server.begin();
  Serial.printf("starting HTTP Serer on port: %i, %i \n", wsport, port);
  espWebSrv::restRouter();
};

void espWebSrv::restRouter(){
  //server.on("/", handleRoot);     // Route for root / web page
  server.on("/", HTTP_GET, espWebSrv::handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  //server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"

  server.on("/temperature", HTTP_GET, espWebSrv::handleDHTtemp); 
  /* server.on("/temperature", HTTP_GET, [](){
    String t = String(myDHTMQTT::readDHTTemp());
    Serial.printf("API request for temperature: %s \n", t.c_str());    
  });  */


  server.on("/humidity", HTTP_GET,espWebSrv::handleDHThumid);
  /* server.on("/humidity", HTTP_GET, [](){
    String h = String(myDHTMQTT::readDHTHumid());
    Serial.printf("API request for humidity: %s \n", h.c_str());    
  });  */
  
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

};

void espWebSrv::initWebSvr(const int port) {
  // //Initialize Webserver
  // ::ESP8266WebServer server(port);
};

void espWebSrv::WSStatus() {

};

int espWebSrv::getWSPort() {
  Serial.printf("Ws Port: %i \n", config.websvrcfg.port );
  return config.websvrcfg.port;
};

void espWebSrv::handleDHTtemp() {  
  /* char* t = 0;
  sprintf(t, "%.2f", myDHTMQTT::readDHTTemp());
  server.send(200, "text/plain", String(t));
  Serial.printf("API request for temperature: %s \n", myDHTMQTT::readDHTTemp()); */
  String t = String(espDHT::readDHTTemp(true));
  Serial.printf("API request for temperature: %s \n", t.c_str());
  server.send(200, "text/plain", t);
};

void espWebSrv::handleDHThumid() {
  /* char* h = 0;
  sprintf(h, "%.2f", myDHTMQTT::readDHTHumid());
  server.send(200, "text/plain", String(h));
  Serial.printf("API request for humidity: %s \n", myDHTMQTT::readDHTHumid()); */
  String h = String(espDHT::readDHTHumid());
  Serial.printf("API request for humidity: %s \n", h.c_str());
  server.send(200, "text/plain", h);
    
};

void espWebSrv::handleRoot() {
  String s = String(index_html); //Read HTML contents
  server.send(200, "text/html", s); 
  Serial.println("Webpage request received");
  //server.send(200, "text/html", "get Temperature \n get humidity \n");
};

void espWebSrv::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  };
  server.send(404, "text/plain", message);  
};

void espWebSrv::handleClient(){
  server.handleClient();
};