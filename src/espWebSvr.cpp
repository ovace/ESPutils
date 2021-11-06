/*
  @file espWebSvr.cpp


*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

#include "LittleFS.h" // LittleFS is declared

#if defined(ESP8266)  
  #include <ESP8266WebServer.h>  
#elif defined(ESP32)  
  #include <WebServer.h>
#else
  #error Invalid platform
#endif 

#include "index.html"  //include the index.html page
#include "espWebSvr.h"
espWebSrv myWebSvr;
espWebSvrCfg myWebSvrCfg;
#include "espTime.h"
espTime myTime;

WebSvrConfig *websvrcfg, mywebsvrcfg;

static const char* cfgFilename = "/espWebSvrConfig.json";  

#define _DEBUG_ mywebsvrcfg.debug

//Initialize Webserver
// ESP8266WebServer server(wsport);
ESP8266WebServer server(90);

// SwitchData myswitchdata;
// progStats myprogstats;

espWebSrv::espWebSrv() { //Class constructor
};
espWebSrv::~espWebSrv() { //Class destructor
};
void espWebSrv::setup(const int port) {
  // read configuraton file
  myWebSvrCfg.loadConfiguration();
  if (_DEBUG_) {
  Serial.println("Configuration JSON File");
    myWebSvrCfg.printFile();

    Serial.println("Configuration: ");
    myWebSvrCfg.printCFG();
  }

  // espWebSrv::initWebSvr(port);  
  // Serial.printf("starting HTTP Serer on port: %i, %i \n", wsport, port);
  Serial.printf("starting HTTP Serer on port:  %i ",  port); Serial.println();
  server.begin();
  myWebSvr.restRouter();
};
void espWebSrv::restRouter(){
   server.on("/", HTTP_GET, espWebSrv::handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
 
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(espWebSrv::handleNotFound);

};
void espWebSrv::initWebSvr(const int port) {
  // //Initialize Webserver
  // ::ESP8266WebServer server(port);
};
void espWebSrv::status() {

};
int espWebSrv::getWSPort() {
  Serial.printf("Ws Port: %i \n", mywebsvrcfg.websvrcfg.port );
  return mywebsvrcfg.websvrcfg.port;
};
void espWebSrv::handleRoot() {
  String s = String(index_html); //Read HTML contents
  server.send(200, "text/html", s); 
  Serial.println("Webpage request received");
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

espWebSvrCfg::espWebSvrCfg() { //Class constructor
 
};
espWebSvrCfg::~espWebSvrCfg() { //Class destructor

};
void espWebSvrCfg::loadConfiguration(const char *cfgFilename, WebSvrConfig &mywebsvrcfg){
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
    Serial.print(F("Opening mywebsvrcfg file: "));
    Serial.println(cfgFilename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(cfgFilename); 
      return;
      };

    const int capacity =   JSON_OBJECT_SIZE(1) + 
                           JSON_OBJECT_SIZE(2) +
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
    mywebsvrcfg.cfg_rel = doc["release"] | 99.99;
    mywebsvrcfg.debug = doc["debug"] | false;

    //WebSvr
    mywebsvrcfg.websvrcfg.port = doc["WebSvr"]["port"] |  80; 
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("Failed not found: ")); Serial.println(cfgFilename); 
    return;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return;
};
void espWebSvrCfg::loadConfiguration(){
  myWebSvrCfg.loadConfiguration(cfgFilename, mywebsvrcfg);
};
void espWebSvrCfg::saveConfiguration(const char *cfgFilename, const WebSvrConfig &mywebsvrcfg) {
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


  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();
};
void espWebSvrCfg::saveConfiguration(){
  myWebSvrCfg.saveConfiguration(cfgFilename, mywebsvrcfg);
};
void espWebSvrCfg::printFile(const char *cfgFilename){
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
void espWebSvrCfg::printFile(){
  myWebSvrCfg.printFile(cfgFilename);
};
void espWebSvrCfg::printCFG(){
  Serial.printf("Release: %f ",mywebsvrcfg.cfg_rel);Serial.println();
  Serial.printf("Debug mode: %s ", mywebsvrcfg.debug? "true"  : "false");Serial.println();
  
  Serial.println(F("WebServer Config"));
  Serial.println(mywebsvrcfg.websvrcfg.port);
};

