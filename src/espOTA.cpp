/*

    @file=espOTA.cpp

    Provides OTA update pull capabilitis vis a Server

    add following to the setup() in your main.cpp

    espOTA::setup();

    add following to the loop in you main.cpp

    espOTA::handle();

    @dependency=espWiFi 

*/
#include <Arduino.h>
#include <string>

#include "LittleFS.h" // LittleFS is declared
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#ifdef ESP32
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
  // #include <WiFi.h>
#elif defined (ESP8266)
  #include <ESP8266HTTPClient.h>
  #include <ESP8266httpUpdate.h>
  // #include <ESP8266WiFi.h>
#else 
  #error "For ESP boards only"
#endif

#include "espOTA.h"
extern espOTA myOta;
espOTAcfg myOtaCfg;

static String code_rel = "0.01 "; // espOTA project

OTAconfig *otaCfg, myotacfg;
// mqttCfg = &myotacfg;

static const char *cfgFilename = "/espOTAconfig.json";  

#define _DEBUG_ myotacfg.debug


espOTA::espOTA() { //Class constructor

};
espOTA::~espOTA() { //Class destructor

};

/* Watchdog to guard against the ESP8266 wasting battery power looking for 
 *  non-responsive wifi networks and servers. Expiry of the watchdog will trigger
 *  either a deep sleep cycle or a delayed reboot. The ESP8266 OS has another built-in 
 *  watchdog to protect against infinite loops and hangups in user code. 
 */
#include <Ticker.h>
Ticker watchdog;
#define WATCHDOG_SETUP_SECONDS  30     // Setup should complete well within this time limit
#define WATCHDOG_LOOP_SECONDS   20    // Loop should complete well within this time limit


// Optional functionality. Comment out defines to disable feature
#define ARDUINO_OTA                   // Enable Arduino IDE OTA updates
#define HTTP_OTA                      // Enable OTA updates from http server
// #define LED_STATUS_FLASH              // Enable flashing LED status

#ifdef LED_STATUS_FLASH

  #define STATUS_LED  2                 // Built-in blue LED on pin 2
  Ticker flasher;

  void flash() {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
  }
#endif

#ifdef ARDUINO_OTA
  /* Over The Air updates directly from Arduino IDE */
  // #include <ESP8266mDNS.h>
  // #include <WiFiUdp.h>
  #include <ArduinoOTA.h>

  #define ARDUINO_OTA_PORT      myotacfg.otacfg.port
  #define ARDUINO_OTA_HOSTNAME  WiFi.hostname().c_str()
  #define ARDUINO_OTA_PASSWD    myotacfg.otacfg.pswd.c_str()
#endif

#ifdef HTTP_OTA
  /*  Over The Air automatic firmware update from a web server.  ESP8266 will contact the
   *  server on every boot and check for a firmware update.  If available, the update will
   *  be downloaded and installed.  Server can determine the appropriate firmware for this 
   *  device from any combination of HTTP_OTA_VERSION, MAC address, and firmware MD5 checksums.
   */
  #define HTTP_OTA_ADDRESS      myotacfg.otacfg.update_server       // Address of OTA update server
  #define HTTP_OTA_PATH         myotacfg.otacfg.url                 // Path to update firmware
  #define HTTP_OTA_PORT         myotacfg.otacfg.update_server_port  // Port of update server  
  #define CUR_FS_VER            myotacfg.cfg_rel                    // Current myotacfg file version
  // #define CUR_FW_VER            code_rel                           // Current Firmware version                      
 
  #define HTTP_OTA_FIRMWARE      "firmware"   // Name of firmware
  #define HTTP_OTA_FS            "littlefs"   // Filesystem to copy to flash -- can not copy individual files, have to refresh full filesystem. Takes a while!
  #define DEVICE_MAC             WiFi.macAddress()
#endif


void timeout_cb() {
  // This sleep happened because of timeout. Do a restart after a sleep
  Serial.println(F("Watchdog timeout..."));

#ifdef DEEP_SLEEP_SECONDS
  // Enter DeepSleep so that we don't exhaust our batteries by countinuously trying to
  // connect to a network that isn't there. 
  ESP.deepSleep(DEEP_SLEEP_SECONDS * 1000, WAKE_RF_DEFAULT);
  // Do nothing while we wait for sleep to overcome us
  while(true){};

#else
  delay(1000);
  Serial.println(F("resetting because of Update timeout"));
  ESP.restart();
#endif
}
void espOTA::setup(String CUR_FW_VER){  
  // read configuraton file
  myOtaCfg.loadConfiguration();

  if (_DEBUG_) {
    Serial.printf("Chip ID: %s \n", String(ESP.getChipId()).c_str()); Serial.println();
    Serial.printf("Device Vcc: %s \n", String(ESP.getVcc()).c_str()); Serial.println();
    Serial.println("MAC: " + DEVICE_MAC); 
    
    Serial.println("Configuration JSON File");
    myOtaCfg.printFile();

    Serial.println("Configuration: ");
    myOtaCfg.printCFG();   
  }

  #ifdef LED_STATUS_FLASH
    pinMode(STATUS_LED, OUTPUT);
    flasher.attach(0.6, flash);
  #endif

  // Watchdog timer - resets if setup takes longer than allocated time
  watchdog.once(WATCHDOG_SETUP_SECONDS, &timeout_cb);

  #ifdef HTTP_OTA
    espOTA::OTA_pull(CUR_FW_VER);
  #endif

  #ifdef ARDUINO_OTA
    espOTA::OTA_push();
  #endif

  #ifdef LED_STATUS_FLASH
    flasher.detach();
    digitalWrite(STATUS_LED, HIGH);
  #endif

  Serial.println(F("Ready"));
  watchdog.detach();

}
void espOTA::OTA_pull(String CUR_FW_VER) {
  #ifdef HTTP_OTA
    // Check server for firmware updates
      Serial.print(F("Checking for firmware updates from server http://"));
      Serial.print(HTTP_OTA_ADDRESS);
      Serial.print(F(":"));
      Serial.print(HTTP_OTA_PORT);
      Serial.println(HTTP_OTA_PATH);
      // Serial.print(F("/"));
      // // Serial.print(F("Firmware Version: "));
      // Serial.println(HTTP_OTA_FIRMWARE);

    if (_DEBUG_) {
      Serial.println("MAC Addr: " +  DEVICE_MAC);
      Serial.printf("MAC ID: %s \n", espOTA::MACADDR2MACID(DEVICE_MAC.c_str()).c_str());
      Serial.println();
      Serial.println("\nUpdate LittleFS...");
    }

      

      WiFiClient client;
      t_httpUpdate_return ret;
      String base_url, url, cur_ver, file;

       // Add optional callback notifiers
      ESPhttpUpdate.onStart(espOTA::update_started);
      ESPhttpUpdate.onEnd(espOTA::update_finished);
      ESPhttpUpdate.onProgress(espOTA::update_progress);
      ESPhttpUpdate.onError(espOTA::update_error);

      Serial.println(F("\n Update flash..."));
      base_url = "http://" + String(HTTP_OTA_ADDRESS) + ":" + int(HTTP_OTA_PORT) + String(HTTP_OTA_PATH);      
      cur_ver = String(CUR_FS_VER);
      file = String(HTTP_OTA_FS) + "-" + espOTA::MACADDR2MACID(DEVICE_MAC.c_str()) + ".bin";
      url = base_url + "/" + file ;
      if (_DEBUG_) {
        Serial.println(url);
        Serial.println(file);
        Serial.println(cur_ver);
      };
      ret = ESPhttpUpdate.updateFS(client, url, cur_ver); //(WiFiClient& client, const String& url, const String& currentVersion)      
      if ( _DEBUG_ ) {
        Serial.printf("HTTP_UPDATE status: %i \n", ret);
        Serial.println();
      };
      switch (ret) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            Serial.println();
            break;

          case HTTP_UPDATE_NO_UPDATES:
            Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
            break;

          case HTTP_UPDATE_OK:
            Serial.println(F("HTTP_UPDATE_OK"));
            break;
      };

      Serial.printf("\n Update sketch...");  
      url = base_url + "/" + String(HTTP_OTA_FIRMWARE) ;
      cur_ver = String(CUR_FW_VER);
      file = String(HTTP_OTA_FIRMWARE) + "-" + espOTA::MACADDR2MACID(DEVICE_MAC.c_str()) + ".bin";
      url = base_url + "/" + file ;

      if ( _DEBUG_ ) {
        Serial.println(url);
        Serial.println(cur_ver);
      };

      ret = ESPhttpUpdate.updateFS(client, url, cur_ver); //(WiFiClient& client, const String& url, const String& currentVersion)
      // Or:
      // ret = ESPhttpUpdate.update(client, String(HTTP_OTA_ADDRESS), int(HTTP_OTA_PORT), (String(HTTP_OTA_PATH) + "/" + String(HTTP_OTA_FIRMWARE)), cur_ver ); //ESPhttpUpdate.update(client, "server", 80, "file.bin");
      // Or:
      //t_httpUpdate_return ret = ESPhttpUpdate.update(client, url, current_version);
      // Or:
      //t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.0.2", 80, "/esp/update/arduino.php", "optional current version string here");
      if ( _DEBUG_ ) {
        Serial.printf("HTTP_UPDATE status: %i \n", ret);
        Serial.println();
      };
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); 
        Serial.println();
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
        break;

      case HTTP_UPDATE_OK:
        Serial.println(F("HTTP_UPDATE_OK"));
        break;
    }
  #endif
  }
void espOTA::OTA_push() {
  #ifdef ARDUINO_OTA
    // Arduino OTA Initalisation
    ArduinoOTA.setPort(ARDUINO_OTA_PORT);    // Port defaults to 8266
    ArduinoOTA.setHostname(ARDUINO_OTA_HOSTNAME);   // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setPassword(ARDUINO_OTA_PASSWD); // No authentication by default
    // Password can be set with it's md5 value as well  
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3  
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");  

    ArduinoOTA.onStart([]() {
        watchdog.detach();
        String type;  
        if (ArduinoOTA.getCommand() == U_FLASH) {  
            type = "sketch";  
        }   
        else { // U_SPIFFS  
            type = "filesystem";  
        }  
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()  
        Serial.println("OTA Start updating " + type);       
    });
    ArduinoOTA.onEnd([]() {
        Serial.println(F("\nOTA End"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        Serial.println();
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
    });
    ArduinoOTA.begin();
    Serial.printf("OTA Ready on  hostname: %s\n", ArduinoOTA.getHostname().c_str());
    Serial.println();
  #endif
};
void espOTA::OTA_push_handle() {
  ArduinoOTA.handle();
};
void espOTA::update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}
void espOTA::update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}
void espOTA::update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\r", cur, total);
  Serial.println();
}
void espOTA::update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
  Serial.println();
}
String espOTA::MACADDR2MACID(String addr)
{
    addr.replace(":", "");
    //addr.replace(.*(?=.{6}$), "");  // Arduino and C++ dont support regex. C++ has a regex library.

    String str = addr.substring(6,12);
    return str;
}


espOTAcfg::espOTAcfg() { //Class constructor
 
};
espOTAcfg::~espOTAcfg() { //Class destructor

};
void espOTAcfg::loadConfiguration(const char *cfgFilename, OTAconfig &myotacfg){
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
    Serial.print(F("Opening myotacfg file: "));
    Serial.println(cfgFilename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(cfgFilename); 
      return;
      };

    const int capacity =   JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(4) + 
                           JSON_OBJECT_SIZE(7) +                            
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
    myotacfg.cfg_rel = doc["release"] | 99.99;
    myotacfg.debug = doc["debug"] | false;

    //OTA
    JsonObject OTA = doc[OTA];
    myotacfg.otacfg.port = OTA["port"] | 80;    
    myotacfg.otacfg.update_server = OTA["update_server"] | "";
    myotacfg.otacfg.url = OTA["url"] | "";
    myotacfg.otacfg.update_server_port = OTA["update_server_port"] | 80;
    myotacfg.otacfg.secret = OTA["secret"] | "";
    myotacfg.otacfg.user = OTA["user"] | "";
    myotacfg.otacfg.pswd = OTA["pswd"] | "";
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("Failed not found: ")); Serial.println(cfgFilename); 
    return;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return;
};
void espOTAcfg::loadConfiguration(){
  myOtaCfg.loadConfiguration(cfgFilename, myotacfg);
};
void espOTAcfg::saveConfiguration(const char *cfgFilename, const OTAconfig &myotacfg) {
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
void espOTAcfg::saveConfiguration(){
  myOtaCfg.saveConfiguration(cfgFilename, myotacfg);
};
void espOTAcfg::printFile(const char *cfgFilename){
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
void espOTAcfg::printFile(){
  myOtaCfg.printFile(cfgFilename);
};
void espOTAcfg::printCFG(){
  Serial.printf("Release: %f ",myotacfg.cfg_rel); Serial.println();
  Serial.printf("Debug mode: %s ", myotacfg.debug? "true"  : "false"); Serial.println();
  
  Serial.println(F("OTA Config"));
  Serial.println(myotacfg.otacfg.port);
  Serial.println(myotacfg.otacfg.pswd);
  Serial.println(myotacfg.otacfg.secret);
  Serial.println(myotacfg.otacfg.update_server);
  Serial.println(myotacfg.otacfg.update_server_port);
  Serial.println(myotacfg.otacfg.url);
  Serial.println(myotacfg.otacfg.user);
};


