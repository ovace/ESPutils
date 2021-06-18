#ifndef __webserver_H
#define __webserver_H

#include "espWebSvr.h"

    class espWebSrv {
        public:
            espWebSrv();
            ~espWebSrv();            
            static void setupWebSvr(const int port);            
            static void WSStatus();   
            static void handleClient();
            static int getWSPort();
            

        private:
            static void initWebSvr(const int port) ;              
            static void restRouter();            
            static void handleRoot(); 
            static void handleNotFound();  
    };    

    class espSwitch {
        public:
            espSwitch();
            ~espSwitch();            
            static void handleSwitchOFF();
            static void handleSwitchON();   
            static void handleGenericArgs();
            static void handleSpecificArg();
            static void handleSwitchOps();

        private:
    };  
#endif