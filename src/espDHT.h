#ifndef _DHT_H
#define _DHT_H

#include "espDHT.h"

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

    
    class espDHT {
        public:
            espDHT();
            ~espDHT();
            static void DHTsetup() ; 
            static float readDHTTemp(bool isFarenheit=false);            
            static float readDHTHumid();
            static sensorReady sensorlatency(unsigned long previousMillis, int dly);   
            
        private:
            void initDHT() ;    
            int getMinimumSamplingPeriod() { return DHTTYPE != DHT11 ? 1000 : 2000; };    
    };    
#endif