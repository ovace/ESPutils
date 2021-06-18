/*

    @file=espOTA.cpp

    Provides OTA update pull capabilitis vis a Server

    add following to the setup() in your main.cpp

    espOTA::setup();

    add following to the loop in you main.cpp

    espOTA::handle();

    @dependency=espWiFi 

*/

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
#include "getCfg.h"
#include "config.h"

espOTA::espOTA() { //Class constructor

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
#define HTTP_OTA                         // Enable OTA updates from http server
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

  #define ARDUINO_OTA_PORT      config.otacfg.port
  #define ARDUINO_OTA_HOSTNAME  config.wirelesscfg.hostname.c_str()
  #define ARDUINO_OTA_PASSWD    config.otacfg.pswd.c_str()
#endif

#ifdef HTTP_OTA
  /*  Over The Air automatic firmware update from a web server.  ESP8266 will contact the
   *  server on every boot and check for a firmware update.  If available, the update will
   *  be downloaded and installed.  Server can determine the appropriate firmware for this 
   *  device from any combination of HTTP_OTA_VERSION, MAC address, and firmware MD5 checksums.
   */
  #define HTTP_OTA_ADDRESS      config.otacfg.update_server       // Address of OTA update server
  #define HTTP_OTA_PATH         config.otacfg.url                 // Path to update firmware
  #define HTTP_OTA_PORT         config.otacfg.update_server_port  // Port of update server  
  #define CUR_FS_VER            config.cfg_rel                    // Current config file version
  #define CUR_FW_VER            code_rel                           // Current Firmware version                      
 
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

void espOTA::setup(){  
  if (_DEBUG_) {
    Serial.printf("Chip ID: %s \n", String(ESP.getChipId()).c_str());
    Serial.printf("Device Vcc: %s \n", String(ESP.getVcc()).c_str());    
    Serial.println();
    Serial.print("MAC: ");
    Serial.println(DEVICE_MAC);
  }

  #ifdef LED_STATUS_FLASH
    pinMode(STATUS_LED, OUTPUT);
    flasher.attach(0.6, flash);
  #endif

  // Watchdog timer - resets if setup takes longer than allocated time
  watchdog.once(WATCHDOG_SETUP_SECONDS, &timeout_cb);

  #ifdef HTTP_OTA
    espOTA::OTA_pull();
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

void espOTA::OTA_pull() {
  #ifdef HTTP_OTA
    // Check server for firmware updates
      Serial.print(F("Checking for firmware updates from server http://"));
      Serial.print(HTTP_OTA_ADDRESS);
      Serial.print(F(":"));
      Serial.print(HTTP_OTA_PORT);
      Serial.print(HTTP_OTA_PATH);
      // Serial.print(F("/"));
      // // Serial.print(F("Firmware Version: "));
      // Serial.println(HTTP_OTA_FIRMWARE);

    if (_DEBUG_) {
      Serial.printf("MAC Addr: %s \n", DEVICE_MAC.c_str());
      Serial.printf("MAC ID: %s \n", espOTA::MACADDR2MACID(DEVICE_MAC.c_str()).c_str());
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
      };
      switch (ret) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
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
      };
      switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); 
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
}

void espOTA::update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

String espOTA::MACADDR2MACID(String addr)
{
    addr.replace(":", "");
    //addr.replace(.*(?=.{6}$), "");  // Arduino and C++ dont support regex. C++ has a regex library.

    String str = addr.substring(6,12);
    return str;
}

