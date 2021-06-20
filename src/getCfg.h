#ifndef _GET_CFG_H
#define _GET_CFG_H

// Our configuration structure.
//
// Never use a JsonDocument to store the configuration!
// A JsonDocument is *not* a permanent storage; it's only a temporary storage
// used during the serialization phase. See:
// https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/
struct GlobalConfig {
  float cfg_rel;
  bool debug;

  struct Devicecfg {
      String name;
      String type;
      String model;
      String location;
    };
    Devicecfg devicecfg;

  struct Wirelesscfg {
      String hostname;
      int port;
      String ssid;
      char psk[64];
      String espmode;
      int connRetries;
    };
    Wirelesscfg wirelesscfg;

  struct Sensorscfg  {
      bool OTA;
      bool OTApull;
      bool MQTT;
      bool WebSvr;
      bool Temperature;
      bool Humidity;
      bool Light;
      bool Soil_moisture;
      bool Soil_salt;
      bool Battery;
      bool Level;
      bool Water;
    };
    Sensorscfg sensorscfg;

  struct MQTTcfg  {
      String server;
      int port = 1883;
      String user;
      String pas;
      String home_topic_base;
    };
    MQTTcfg mqttcfg;

  struct NTPcfg{
      String Server;
      int refrehFreq;
      char timezone;
    };
    NTPcfg ntpcfg;

  struct WebSvrcfg {
      int port;
    };
    WebSvrcfg websvrcfg;

  struct SerialComcfg {
      int espbaud;
    };
    SerialComcfg serialcomcfg;

  struct Timerscfg {
      int MEASUREMENT_TIMEINTERVAL;
      int TIME_TO_SLEEP;
          };
    Timerscfg timerscfg;

  struct DHTcfg {
      int pin ;
      int type; 
    };
    DHTcfg dhtcfg;

  struct OTAcfg {
      int port;
      String update_server;
      String url;
      int update_server_port;
      String secret;
      String user;
      String pswd;
   };
   OTAcfg otacfg;

};
extern GlobalConfig  config;                         // <- global configuration object -- cfg is the pointer 

  class espCFG {
        public:
            espCFG();
            //~espCFG();
            void loadConfiguration(const char *filename, GlobalConfig &config);
            void loadConfiguration();
            void saveConfiguration(const char *filename, const GlobalConfig &config) ;
            void saveConfiguration();
            void printFile(const char *filename);
            void printFile();
            void printCFG();
            
        private:
            
    };

#endif //_GET_CFG_H