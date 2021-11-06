#ifndef _esp_MQTT_WiFi_H
#define _esp_MQTT_WiFi_H

    #include <AsyncMqttClient.h>
   
    // Our configuration structure.
    //
    // Never use a JsonDocument to store the configuration!
    // A JsonDocument is *not* a permanent storage; it's only a temporary storage
    // used during the serialization phase. See:
    // https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/
    struct MQTTconfig {
        float cfg_rel;
        bool debug;
          struct MQTTcfg  {
            String server;
            int port = 1883;
            String user;
            String pas;
            String home_topic_base;
            };    
            MQTTcfg mqttcfg;        
        };

    class espMQTT {
        public:
            espMQTT();
            ~espMQTT();
            static void setup();
            static int mqttpub(const char* TOPIC, const char* PAYLOAD);
            static int mqttpub(const char* TOPIC, uint8_t qos, bool retain, const char* PAYLOAD);
            static void connectToMqtt() ;
            
        private:
            static void initMQTT();
            static void onMqttConnect(bool sessionPresent) ;
            static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
            static void onMqttSubscribe(uint16_t packetId, uint8_t qos) ;
            static void onMqttUnsubscribe(uint16_t packetId) ;
            static void onMqttPublish(uint16_t packetId);             
    };

    class espMQTTcfg {
            public:
                espMQTTcfg();
                ~espMQTTcfg();
                bool loadConfiguration(const char *cfgFilename, MQTTconfig &mymqttcfg);
                 void loadConfiguration();
                void saveConfiguration(const char *cfgFilename, const MQTTconfig &mymqttcfg) ;
                void saveConfiguration();
                void printFile(const char *cfgFilename);
                void printFile();
                void printCFG();
                
            private:
                
        };
   
#endif