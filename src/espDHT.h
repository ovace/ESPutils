#ifndef _DHT_H
#define _DHT_H

    #include <DHT.h>
    
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

    struct DHTconfig {
    float cfg_rel;
    bool debug;
    struct DHTcfg {
        int pin ;
        int type; 
        };
        DHTcfg dhtcfg;
    };
    
    class espDHT {
        public:
            espDHT();
            ~espDHT();
            static void setup() ; 
            static float readTemp(bool isFarenheit=false);            
            static float readHumid();
            static sensorReady sensorlatency(unsigned long previousMillis, int dly);   
            
        private:
            static void initDHT() ;    
            static int getMinimumSamplingPeriod();
    };

    class espDHTcfg {
            public:
                espDHTcfg();
                ~espDHTcfg();
                void loadConfiguration(const char *cfgFilename,DHTconfig &mydhtcfg);
                void loadConfiguration();
                void saveConfiguration(const char *cfgFilename, const DHTconfig &mydhtcfg) ;
                void saveConfiguration();
                void printFile(const char *cfgFilename);
                void printFile();
                void printCFG();
                
            private:
                
        };
    
#endif