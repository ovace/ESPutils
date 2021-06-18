#ifndef _esp_MQTT_WiFi_H
#define _esp_MQTT_WiFi_H

    #include <AsyncMqttClient.h>
    #include "config.h"

    struct Switch_Data {
        String SW_Name;  //variable for Switch Name
        bool SW_State;  //Current state of Switch

        int error;  // error value 0 --> no error 1--> error
    };

    struct sensorReady{
        bool ready;
        unsigned long previosMillis;
    };

    struct wifiStat {        
        const char* mac;
        const char* ssid;
        IPAddress ip;        
        const char* hostname;
        int status;
        float rssi;
        bool connState;
    };

    class espWiFi {
        public:
            espWiFi();
            ~espWiFi();
            static void wifiSetup(); 
            static wifiStat wifiStatus() ;
            static void connectToWifi();            

        private:
            static void initWifi() ;
            static void onWifiConnect(const WiFiEventStationModeGotIP& event);
            static void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);

    };

    class espMQTT {
        public:
            espMQTT();
            ~espMQTT();
            static void mqttSetup();
            static void mqttpub(const char* TOPIC, const char* PAYLOAD);
            static void connectToMqtt() ;
            
        private:
            static void initMQTT();
            static void onMqttConnect(bool sessionPresent) ;
            static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
            static void onMqttSubscribe(uint16_t packetId, uint8_t qos) ;
            static void onMqttUnsubscribe(uint16_t packetId) ;
            static void onMqttPublish(uint16_t packetId);   
    };

    class espSensor {
        public:   
            espSensor();
            ~espSensor();
            static sensorReady sensorlatency(unsigned long previousMillis, int dly);   
    };    
       
#endif