#ifndef _GLOBAL_VARS
#define _GLOBAL_VARS

static String code_rel = "0.16 "; // ESP sensor project


#define _DEBUG_ config.debug

#include <getCfg.h>

// extern GlobalConfig config;

extern espCFG mycfg;

static void cfgBegin() {
    mycfg.loadConfiguration();  
    return;
};

// These are the configirable parameters for the ESP
   
    #ifndef STASSID        
        #define esp_mode WIFI_STA     //WIFI_STA=1, WIFI_AP=2 or WIFI_AP_STA=3           
        #define DHTTYPE DHT11      
        // const int DHTTYPE = 11;  
        static const int DHTPIN = 2;
        #define conRetry config.wirelesscfg.connRetries
        static int wsport = 90;
    #endif

    #ifndef LOADMOD
        #define doOTA config.sensorscfg.OTA
        #define doOTApull true
        #define snsTemp config.sensorscfg.Temperature
        #define snsHumid config.sensorscfg.Humidity
        #define snsLux  config.sensorscfg.Light
        #define snsSoilMoist config.sensorscfg.Soil_moisture
        #define snsSoilSlt config.sensorscfg.Soil_salt
        #define snsBatt config.sensorscfg.Battery
        #define snsLvl config.sensorscfg.Level
        #define snsWater config.sensorscfg.Water
        #define doMQTT config.sensorscfg.MQTT
        #define doWebSvr config.sensorscfg.WebSvr
    #endif //LOADMOD

    // static const char* ssid = config.wirelesscfg.ssid;
    // static const char* password = config.wirelesscfg.psk;
    // static const char* host_name = config.wirelesscfg.hostname;
    // static const int conRetries = config.wirelesscfg.connRetries;
    

    // static const int wsport = config.websvrcfg.port;

    // email server settings
    static const int     SMTP_PORT            = 465;
    static const char*   SMTP_SERVER          = "smtp.gmail.com";

    /*
    * 
    * if you are NOT using GMAIL then change the
    * above SMTP_PORT and SMTP_Server to match
    * your email provider. 
    * 
    */
        static String Senders_Login    = "xxxxx";  // your email provider login
        static String Senders_Password = "xxxxx";  // your email provider password
        static String From;


   
// *******************************************************************************************************************************
// END userdefined data
// *******************************************************************************************************************************


#endif