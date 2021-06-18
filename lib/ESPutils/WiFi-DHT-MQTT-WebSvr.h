#ifndef _DHT_MQTT_webserver_H
#define _DHT_MQTT_webserver_H

#include "WiFi-DHT-MQTT-WebSvr.h"

    #include <AsyncMqttClient.h>
    #include <DHT.h>
    #include "config.h"

    struct Sensor_Data {
        float temp_c ; // variable for temperature in Celcius
        float temp_f ; // variable for temperature in Farenheit
        float humid; // variable for humidity 
        float hi_c;    //variable for heat index in Celcius
        float hi_f;    //variable for heat index in Farenheit
        float moist;  //variable for moisture
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
            //~espWiFi();
            static void wifiSetup(); 
            static wifiStat wifiStatus() ;
            static void connectToWifi();

        private:
            static void initWifi();
            static void onWifiConnect(const WiFiEventStationModeGotIP& event);
            static void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);
    };

    class espMQTT {
        public:
            espMQTT();
            //~espMQTT();
            static void mqttSetup();            
            void mqttpub(const char* TOPIC, const char* PAYLOAD);
            static void connectToMqtt() ; 
            

        private:
           static void initMQTT(); 

            static void onMqttConnect(bool sessionPresent) ;
            static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
            static void onMqttSubscribe(uint16_t packetId, uint8_t qos) ;
            static void onMqttUnsubscribe(uint16_t packetId) ;
            static void onMqttPublish(uint16_t packetId);           
    };

    class espDHT {
        public:
            espDHT();
            //~espDHT();
            static void DHTsetup() ; 
            static float readDHTTemp(bool isFarenheit=false);            
            static float readDHTHumid();
            static sensorReady sensorlatency(unsigned long previousMillis, int dly);   
            
        private:
            static void initDHT() ;    
            static int getMinimumSamplingPeriod() { return DHTTYPE != DHT11 ? 1000 : 2000; };    
    };

    class espWebSrv {
        public:
            espWebSrv();
            //~espWebSrv();            
            static void setupWebSvr(const int port);            
            static void WSStatus();   
            static void handleClient();
            static int getWSPort();
            

        private:
            static void initWebSvr(const int port) ;              
            static void restRouter();
            static void handleDHTtemp();
            static void handleDHThumid();
            static void handleRoot(); 
            static void handleNotFound();
            
    };    

  
#endif