#ifndef __espRelay_H
#define __espRealy_H

#include "SwitchData.h"

    class espRelay {
        public:
            espRelay();
            ~espRelay();            
            static void setupRelay();            
            static bool RelayStatus();
            static bool relayClose();
            static bool relayOpen();  
        
        private:
    };  

    class espSend {
        public:
        espSend();
        ~espSend();
        static void saveSensorData(const SwitchData& switchdata);
        static void sendData(SwitchData& switchdata);
        static SwitchData prepData(); 
        using JsonDocument = StaticJsonDocument<1024>;
        StaticJsonDocument<1024> createJSON(SwitchData & switchdata);
        static void blinkled(int repeat, int intrvlON, int intrvlOFF);
        static bool sendMsg();
        static void sendMQTT(const char* topic, StaticJsonDocument<1024>& JsonDoc);
        static void sendMQTT(const char* topic, String payload);
    };
  
#endif