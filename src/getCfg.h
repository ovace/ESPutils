#ifndef _GET_CFG_H
#define _GET_CFG_H

// Our configuration structure.
//
// Never use a JsonDocument to store the configuration!
// A JsonDocument is *not* a permanent storage; it's only a temporary storage
// used during the serialization phase. See:
// https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/

/* these are the configuration variables that are used in various files.
Any changes here should be reflected in config.h and getCfg.cpp

*/

struct GlobalConfig {
  float cfg_rel;
  bool debug;

struct Devicecfg {
    String name;
    String type;
    String model;
    String location;
    String place;
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

struct SensorsOptcfg  {
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
  SensorsOptcfg sensorsoptcfg;

  struct SensorCfg {
  String date;
  String time;
  String tz;
  int bootno;
  int sleep5no;
  float lux;
  float temp;
  float humid;
  float soil;
  float salt;
  String saltadvice;
  float bat;
  String batcharge;
  float batvolt;
  float batvoltage;
  String code_rel;
};
 SensorCfg sensorcfg;

//  sensorcfg.tz = ntpcfg.timeZone;

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
    String timeZone;
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
          ~espCFG();
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