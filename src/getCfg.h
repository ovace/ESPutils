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

  struct ModuleCfg  {
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
      bool ADC;
    };
    ModuleCfg moduleCfg;

  // struct NTPcfg{
  //     String Server;
  //     int refrehFreq;
  //   };
  //   NTPcfg ntpcfg;

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
};
extern GlobalConfig  config;


class espCFG {
      public:
          espCFG();
          ~espCFG();
          bool setup();
          GlobalConfig loadConfiguration(const char *filename, GlobalConfig &config);
          GlobalConfig loadConfiguration();
          bool saveConfiguration(const char *filename, const GlobalConfig *config) ;
          void saveConfiguration();
          void printFile(const char *filename);
          void printFile();
          void printCFG();
          
      private:
          
  };

#endif //_GET_CFG_H