#ifndef __webserver_H
#define __webserver_H

struct WebSvrConfig {
  float cfg_rel;
  bool debug;

  struct WebSvrcfg {
      int port;
    };
    WebSvrcfg websvrcfg; 
};
// extern WebSvrConfig  config;  // <- global configuration object -- cfg is the pointer 

#include "espWebSvr.h"

    class espWebSrv {
        public:
            espWebSrv();
            ~espWebSrv();            
            static void setup(const int port);            
            static void status();   
            static void handleClient();
            static int getWSPort();            

        private:
            void initWebSvr(const int port) ;              
            static void restRouter();            
            static void handleRoot(); 
            static void handleNotFound();  
    };

    class espWebSvrCfg {
        public:
            espWebSvrCfg();
            ~espWebSvrCfg();
            void loadConfiguration(const char *filename,  WebSvrConfig &mywebsvrcfg);
            void loadConfiguration();
            void saveConfiguration(const char *filename, const  WebSvrConfig &mywebsvrcfg) ;
            void saveConfiguration();
            void printFile(const char *filename);
            void printFile();
            void printCFG();
            
        private:
            
    };  
#endif