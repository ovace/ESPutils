#pragma once

#ifndef __espNWconn_H
#define __espNWconn_H

// struct wifiStat {        
//         const char* mac;
//         const char* ssid;
//         IPAddress ip;        
//         const char* hostname;
//         int status;
//         float rssi;
//         bool connState;
//     };

struct NWconfig {
  float cfg_rel;
  bool debug;  
  struct Wirelesscfg {
      String hostname;
      int port;
      String ssid;
    //   char psk[64];
      String psk;
      String espmode;
      int connRetries;
    };
    Wirelesscfg wirelesscfg;
};

class espNWconn {
    public:
        espNWconn();
        ~espNWconn();            
        static void setup(); 
        static void getCfg();
        static void connect();
        static void WiFiOff();
        static void WiFiOn();
        static void WiFiSleep();
        static void WiFiAwake();
        
    private:
};  

class espNWcfg {
            public:
                espNWcfg();
                ~espNWcfg();
                bool loadConfiguration(const char *cfgFilename,NWconfig &mynwcfg);
                void loadConfiguration();
                void saveConfiguration(const char *cfgFilename, const NWconfig &mynwcfg) ;
                void saveConfiguration();
                void printFile(const char *cfgFilename);
                void printFile();
                void printCFG();
                
            private:
                
        };
#endif