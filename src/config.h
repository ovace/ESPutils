#ifndef _GLOBAL_VARS
#define _GLOBAL_VARS

// static String code_rel = "1.0 "; // ESP sensor project


#define _DEBUG_ config.debug

#include <getCfg.h>
espCFG mycfg;

static void cfgBegin() {
    mycfg.setup();  
};

// These are the configirable parameters for the ESP
   
    #ifndef LOADMOD
        #define doOTA config.moduleCfg.OTA
        #define doOTApull true
        #define snsTemp config.moduleCfg.Temperature
        #define snsHumid config.moduleCfg.Humidity
        #define snsLux  config.moduleCfg.Light
        #define snsSoilMoist config.moduleCfg.Soil_moisture
        #define snsSoilSlt config.moduleCfg.Soil_salt
        #define snsBatt config.moduleCfg.Battery
        #define snsLvl config.moduleCfg.Level
        #define snsWater config.moduleCfg.Water
        #define doMQTT config.moduleCfg.MQTT
        #define doWebSvr config.moduleCfg.WebSvr
        #define doADC config.moduleCfg.ADC
    #endif //LOADMOD


    // // email server settings
    // static const int     SMTP_PORT            = 465;
    // static const char*   SMTP_SERVER          = "smtp.gmail.com";

    // /*
    // * 
    // * if you are NOT using GMAIL then change the
    // * above SMTP_PORT and SMTP_Server to match
    // * your email provider. 
    // * 
    // */
    //     static String Senders_Login    = "xxxxx";  // your email provider login
    //     static String Senders_Password = "xxxxx";  // your email provider password
    //     static String From;


   
// *******************************************************************************************************************************
// END userdefined data
// *******************************************************************************************************************************


#endif