#ifndef _GLOBAL_VARS
#define _GLOBAL_VARS

// static String code_rel = "0.01 "; // ESP sensor project

#define _DEBUG_ config.debug

#include <getCfg.h>

// extern GlobalConfig config;
extern espCFG mycfg;

static void cfgBegin() {
    mycfg.loadConfiguration();  
    return;
};

// These are the configirable parameters for the ESP

// Turn logging on/off - turn read logfile on/off, turn delete logfile on/off ---> default is false for all 3, otherwise it can cause battery drainage.
 static const bool  logging = false;
 static const bool  readLogfile = false;
 static const bool  deleteLogfile = false;
 static String readString; // do not change this variable
   
#ifndef _STASSID_
#define _STASSID_      
    #define esp_mode WIFI_STA     //WIFI_STA=1, WIFI_AP=2 or WIFI_AP_STA=3           
    #define DHTTYPE DHT11      
    // const int DHTTYPE = 11;  
    static const int DHTPIN = 16;
    #define conRetry config.wirelesscfg.connRetries
    static int wsport = 90;
#endif //_STASSID_

#ifndef _LOADMOD_
#define _LOADMOD_
    #define doOTA config.sensorsoptcfg.OTA
    #define doOTApull true
    #define snsTemp config.sensorsoptcfg.Temperature
    #define snsHumid config.sensorsoptcfg.Humidity
    #define snsLux  config.sensorsoptcfg.Light
    #define snsSoilMoist config.sensorsoptcfg.Soil_moisture
    #define snsSoilSlt config.sensorsoptcfg.Soil_salt
    #define snsBatt config.sensorsoptcfg.Battery
    #define snsLvl config.sensorsoptcfg.Level
    #define snsWater config.sensorsoptcfg.Water
    #define doMQTT config.sensorsoptcfg.MQTT
    #define doWebSvr config.sensorsoptcfg.WebSvr
#endif //_LOADMOD_

#ifndef _PROGSTATS_
#define _PROGSTATS_

#endif //_PROGSTATS_

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
   
// ***********************************************
// END userdefined data
// ***********************************************

#endif