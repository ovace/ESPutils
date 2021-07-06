#include <FS.h>
#include <ArduinoJson.h>

#if defined(ESP8266)
 #include "LittleFS.h" // myfs is declared
 LITTLEFS myfs = LITTLEFS;
#elif defined(ESP32)
  #include "SPIFFS.h"
  FS myfs = SPIFFS;
#else
  #error Invalid platform
#endif 


#include "getCfg.h"

GlobalConfig *cfg, config;
// cfg = &config;



const char *filename = "/config.json";  

espCFG::espCFG() { //Class constructor

};
espCFG::~espCFG() { //Class destructor

};


// Loads the configuration from a file
void espCFG::loadConfiguration(const char *filename, GlobalConfig &config) {

  //mounts file system.
  #ifdef ESP32
    Serial.println("\nESP32 Test");
    SPIFFS.begin(true);
  #endif
  #ifdef ESP8266
    Serial.println("\nESP8266 Test");
    myfs.begin();

    
    FSInfo fs_info;
    myfs.info(fs_info);
      Serial.println( fs_info.totalBytes);
      Serial.println(fs_info.usedBytes);
      Serial.println(fs_info.blockSize);
      Serial.println(fs_info.pageSize);
      Serial.println(fs_info.maxOpenFiles);
      Serial.println(fs_info.maxPathLength);
  #endif

  // Open file for reading
  if (myfs.exists(filename)) {

    File file = myfs.open(filename, "r");
    Serial.println(F("Opening config file"));
    if (!file) {
      Serial.println(F("file open failed"));
      return;
      };

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    const int capacity = 2*JSON_OBJECT_SIZE(1) + 
                         3*JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(4) + 
                           JSON_OBJECT_SIZE(5) + 
                           JSON_OBJECT_SIZE(6) + 
                           JSON_OBJECT_SIZE(7) + 
                         2*JSON_OBJECT_SIZE(12) +
                          560 + 250;
    StaticJsonDocument<capacity> doc;
    // SDynamicJsonBuffer Doc<1500>;

    // int jcount = doc.size();
    // Serial.println(F(jcount));

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {  
      Serial.println(F("Failed to read file, using default configuration"));
      return;
    };
    // Copy values from the JsonDocument to the Config

    //Release
    config.cfg_rel = doc["release"] | 99.99;
    config.debug = doc["debug"] | false;

    //Device
    // strlcpy(config.devicecfg.name, doc["Device"]["name"] | "", sizeof(config.devicecfg.name));
    config.devicecfg.name = doc["Device"]["name"] | "";
    // strlcpy(config.devicecfg.type, doc["Device"]["type"] | "esp", sizeof(config.devicecfg.type));
    config.devicecfg.type = doc["Device"]["type"] | "esp";
    // strlcpy(config.devicecfg.model, doc["Device"]["model"] | "esp8266", sizeof(config.devicecfg.model));
    config.devicecfg.model = doc["Device"]["model"] | "esp8266";
    // strlcpy(config.devicecfg.location, doc["Device"]["location"] | "home", sizeof(config.devicecfg.location));
    config.devicecfg.location = doc["Device"]["location"] | "home";
    config.devicecfg.place = doc["Device"]["place"] | "home";
    
    //WiFi
    // strlcpy(config.wirelesscfg.hostname, doc["Device"]["name"] | "esp", sizeof(config.wirelesscfg.hostname));
    config.wirelesscfg.hostname = doc["Device"]["name"] | "esp";
    // strlcpy(config.wirelesscfg.hostname, doc["WiFi"]["hostname"] | "esp", sizeof(config.wirelesscfg.hostname));
    config.wirelesscfg.port =  doc["WiFi"]["port"] | 2731;
    // strlcpy(config.wirelesscfg.ssid, doc["WiFi"]["ssid"] | "", sizeof(config.wirelesscfg.ssid));
    config.wirelesscfg.ssid = doc["WiFi"]["ssid"] | "";
    strlcpy(config.wirelesscfg.psk,  doc["WiFi"]["psk"] | "", sizeof(config.wirelesscfg.psk));
    // config.wirelesscfg.psk = doc["WiFi"]["psk"] | "";
    // strlcpy(config.wirelesscfg.espmode, doc["WiFi"]["espmode"] | "WIFI_STA", sizeof(config.wirelesscfg.espmode));
    config.wirelesscfg.espmode = doc["WiFi"]["espmode"] | "WIFI_STA";
    config.wirelesscfg.connRetries = doc["WiFi"]["connRetries"] |10; 
    
    //Sensors
    config.sensorsoptcfg.OTA =           doc["Sensors"]["OTA"]           | false;
    config.sensorsoptcfg.MQTT =          doc["Sensors"]["MQTT"]          | false;
    config.sensorsoptcfg.WebSvr =        doc["Sensors"]["WebSvr"]        | false;
    config.sensorsoptcfg.Temperature =   doc["Sensors"]["Temperature"]   | false;
    config.sensorsoptcfg.Humidity =      doc["Sensors"]["Humidity"]      | false;
    config.sensorsoptcfg.Light =         doc["Sensors"]["Light"]         | false;
    config.sensorsoptcfg.Soil_moisture = doc["Sensors"]["Soil_moisture"] | false;
    config.sensorsoptcfg.Soil_salt =     doc["Sensors"]["Soil_salt"]     | false;
    config.sensorsoptcfg.Battery =       doc["Sensors"]["Battery"]       | false;
    config.sensorsoptcfg.Level =         doc["Sensors"]["Level"]         | false;
    config.sensorsoptcfg.Water =         doc["Sensors"]["Water"]         | false;    

    //MQTT
    // strlcpy(config.mqttcfg.server, doc["MQTT"]["server"] | "", sizeof(config.mqttcfg.server));
    config.mqttcfg.server =  doc["MQTT"]["server"] | "";
    config.mqttcfg.port = doc["MQTT"]["port"] | 1883;
    // strlcpy(config.mqttcfg.user, doc["MQTT"]["user"] | "", sizeof(config.mqttcfg.user));
    config.mqttcfg.user = doc["MQTT"]["user"] | "";
    // strlcpy(config.mqttcfg.pas, doc["MQTT"]["pas"] | "", sizeof(config.mqttcfg.pas));
    config.mqttcfg.pas =  doc["MQTT"]["pas"] | "";
    // strlcpy(config.mqttcfg.home_topic_base, doc["MQTT"]["home_topic_base"] | "esp/home", sizeof(config.mqttcfg.home_topic_base)); 
    config.mqttcfg.home_topic_base = doc["MQTT"]["home_topic_base"] | "esp/home";

    //NTP
    // strlcpy(config.ntpcfg.Server, doc["NTP"]["Server"] | "pool.ntp.org", sizeof(config.ntpcfg.Server));
    config.ntpcfg.Server = doc["NTP"]["Server"] | "time.nist.gov";
    config.ntpcfg.refrehFreq = doc["NTP"]["refrehFreq"] |  6000; 
    config.ntpcfg.timeZone = doc["NTP"]["timeZone"] |  'UTC0'; 

    //WebSvr
    config.websvrcfg.port = doc["WebSvr"]["port"] |  80; 

    //Serial
    config.serialcomcfg.espbaud = doc["Serial"]["espbaud"] |  115200; 

    //Timers
    config.timerscfg.MEASUREMENT_TIMEINTERVAL  = doc["Timers"]["MEASUREMENT_TIMEINTERVAL"] |  6000; 
    config.timerscfg.TIME_TO_SLEEP  = doc["Timers"]["TIME_TO_SLEEP"] |  6000; 

    //DHT
    config.dhtcfg.pin  = doc["DHT"]["pin"] |  2; 
    config.dhtcfg.type  = doc["DHT"]["type"] |  11; 

    //OTA
    config.otacfg.port = doc["OTA"]["port"] | 80;    
    config.otacfg.update_server = doc["OTA"]["update_server"] | "";
    config.otacfg.url = doc["OTA"]["url"] | "";
    config.otacfg.update_server_port = doc["OTA"]["update_server_port"] | 80;
    config.otacfg.secret = doc["OTA"]["secret"] | "";
    config.otacfg.user = doc["OTA"]["user"] | "";
    config.otacfg.pswd = doc["OTA"]["pswd"] | "";

    

    // strlcpy(config.wirelesscfg.hostname,                  // <- destination
    //         doc["Device"]["name"] | "esp",                // <- source
    //         sizeof(config.wirelesscfg.hostname));         // <- destination's capacity   

    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.println(F("File not found"));
    return;
  };  
  Serial.println("done processing JSON");  

  #ifdef ESP32
    SPIFFS.end();
  #endif
  #ifdef ESP8266
    myfs.end(); //unmounts file system.
    
    FSInfo fs_info;
    myfs.info(fs_info);
      Serial.println( fs_info.totalBytes);
      Serial.println(fs_info.usedBytes);
      Serial.println(fs_info.blockSize);
      Serial.println(fs_info.pageSize);
      Serial.println(fs_info.maxOpenFiles);
      Serial.println(fs_info.maxPathLength);
  #endif
  
  return;
}

void espCFG::loadConfiguration() {
  espCFG::loadConfiguration(filename, config);
}

// Saves the configuration to a file
void espCFG::saveConfiguration(const char *filename, const GlobalConfig &config) {

  //mounts file system.
  #ifdef ESP32    
    SPIFFS.begin(true);
  #endif
  #ifdef ESP8266    
    myfs.begin();
  #endif

  // Delete existing file, otherwise the configuration is appended to the file
  myfs.remove(filename);

  // Open file for writing
  File file = myfs.open(filename, "r+");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["hostname"] = config.wirelesscfg.hostname;
  doc["port"] = config.wirelesscfg.port;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  //unmounts file system.
  #ifdef ESP32    
    SPIFFS.end();
  #endif
  #ifdef ESP8266    
    myfs.end();
  #endif
}

void espCFG::saveConfiguration() {
  espCFG::saveConfiguration(filename, config);
}

// Prints the content of a file to the Serial
void espCFG::printFile(const char *filename) {

  //mounts file system.
    //unmounts file system.
  #ifdef ESP32    
    SPIFFS.end();
  #endif
  #ifdef ESP8266    
    myfs.end();    
  #endif

    //unmounts file system.
  #ifdef ESP32    
    SPIFFS.end();    
  #endif
  #ifdef ESP8266    
    FSInfo fs_info;
    myfs.info(fs_info);        
      Serial.println( fs_info.totalBytes);
      Serial.println(fs_info.usedBytes);
      Serial.println(fs_info.blockSize);
      Serial.println(fs_info.pageSize);
      Serial.println(fs_info.maxOpenFiles);
      Serial.println(fs_info.maxPathLength);
  #endif
  


  // Open file for reading
  File file = myfs.open(filename, "r");
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();

  //unmounts file system.
      //unmounts file system.
  #ifdef ESP32    
    SPIFFS.end();    
  #endif
  #ifdef ESP8266    
    FSInfo fs_info;    
  #endif
}

void espCFG::printFile(){

  espCFG::printFile(filename);

}

void espCFG::printCFG() { 
  Serial.printf("Release: %d \n",config.cfg_rel);
  Serial.printf("Debug mode: %s \n", config.debug? "true"  : "false");

  Serial.println(F("Device Config")); 
  Serial.println(config.devicecfg.name);
  Serial.println(config.devicecfg.type);
  Serial.println(config.devicecfg.model);
  Serial.println(config.devicecfg.location);
 
  Serial.println(F("WiFi Config"));
  Serial.println(config.wirelesscfg.hostname);
  Serial.println(config.wirelesscfg.port);
  Serial.println(config.wirelesscfg.ssid);
  Serial.println(config.wirelesscfg.psk);
  Serial.println(config.wirelesscfg.espmode);
  Serial.println(config.wirelesscfg.connRetries);
    
  Serial.println(F("Sensors options Config"));
  // Serial.println(config.sensorsoptcfg.OTA);
  // Serial.println(config.sensorsoptcfg.MQTT);
  // Serial.println(config.sensorsoptcfg.WebSvr);
  // Serial.println(config.sensorsoptcfg.Temperature);
  // Serial.println(config.sensorsoptcfg.Humidity);
  // Serial.println(config.sensorsoptcfg.Light);
  // Serial.println(config.sensorsoptcfg.Soil_moisture);
  // Serial.println(config.sensorsoptcfg.Soil_salt);
  // Serial.println(config.sensorsoptcfg.Battery);
  // Serial.println(config.sensorsoptcfg.Level);
  // Serial.println(config.sensorsoptcfg.Water);
  Serial.printf("Enable OTA: %s\n", config.sensorsoptcfg.OTA?"Yes":"No");
  Serial.printf("Enable OTA Pull: %s\n", config.sensorsoptcfg.OTApull?"Yes":"No");  
  Serial.printf("MQTT: %s\n", config.sensorsoptcfg.MQTT?"Yes":"No");
  Serial.printf("WebSvr: %s\n", config.sensorsoptcfg.WebSvr?"Yes":"No");
  Serial.printf("Temperature: %s\n", config.sensorsoptcfg.Temperature?"Yes":"No");
  Serial.printf("Humidity: %s\n", config.sensorsoptcfg.Humidity?"Yes":"No");
  Serial.printf("Light: %s\n", config.sensorsoptcfg.Light?"Yes":"No");
  Serial.printf("Soil_Moisture: %s\n", config.sensorsoptcfg.Soil_moisture?"Yes":"No");
  Serial.printf("Soil_salt: %s\n", config.sensorsoptcfg.Soil_salt?"Yes":"No");
  Serial.printf("Battery: %s\n", config.sensorsoptcfg.Battery?"Yes":"No");
  Serial.printf("Level: %s\n", config.sensorsoptcfg.Level?"Yes":"No");
  Serial.printf("Water: %s\n", config.sensorsoptcfg.Water?"Yes":"No");

  Serial.println(F("MQTT Config"));
  Serial.println(config.mqttcfg.server);
  Serial.println(config.mqttcfg.port);
  Serial.println(config.mqttcfg.user);
  Serial.println(config.mqttcfg.pas);
  Serial.println(config.mqttcfg.home_topic_base);

  Serial.println(F("NTP Config"));
  Serial.println(config.ntpcfg.Server);
  Serial.println(config.ntpcfg.refrehFreq);
  Serial.println(config.ntpcfg.timeZone);

  Serial.println(F("WebServer Config"));
  Serial.println(config.websvrcfg.port);

  Serial.println(F("Serial Config"));
  Serial.println(config.serialcomcfg.espbaud);

  Serial.println(F("Timers Config"));
  Serial.println(config.timerscfg.MEASUREMENT_TIMEINTERVAL);
  Serial.println(config.timerscfg.TIME_TO_SLEEP);

  Serial.println(F("DHT Config"));
  Serial.println(config.dhtcfg.pin);
  Serial.println(config.dhtcfg.type);

  Serial.println(F("OTA Config"));
  Serial.println(config.otacfg.port);
  Serial.println(config.otacfg.pswd);
  Serial.println(config.otacfg.secret);
  Serial.println(config.otacfg.update_server);
  Serial.println(config.otacfg.update_server_port);
  Serial.println(config.otacfg.url);
  Serial.println(config.otacfg.user);

}