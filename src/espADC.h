#ifndef _ESP_ADC__H
#define _ESP_ADC__H

    struct ADCdata {
        int ADC[8];
    };
    extern ADCdata adcdata;

    // Our configuration structure.
    //
    // Never use a JsonDocument to store the configuration!
    // A JsonDocument is *not* a permanent storage; it's only a temporary storage
    // used during the serialization phase. See:
    // https://arduinojson.org/v6/faq/why-must-i-create-a-separate-config-object/
    struct ADCconfig {
        float cfg_rel;
        bool debug;
        struct ADCcfg {
            String Type; // MCP3008
            int Bauds; //9600, 119200, 74480, 74880, 115200...
            int numChannels; // nuber of Aanalog channels
            };
            ADCcfg adccfg;
        struct SPIcfg  {
            //define pin connections
            int MISOpin; //MISO -- D7
            int MOSIpin; //MOSI -- D6
            int SCLKpin; // SCLK -- D4
            int CSpin; // CS -- D8        
            };
            SPIcfg spicfg;
        struct UARTcfg {
            int TXpin; //TX -- 3
            int RXpin; //RX -- 1
            };
            UARTcfg uartcfg;
        struct Calib {
            int CHlo[8]; //CH0 low vlaue
            int CHhi[8]; //CH0 Hight value            
            };
            Calib calib;
    };

    class espADC {
        public:
            espADC();
            ~espADC();
            static bool setup();
            static int readChannel(uint8_t channel);
            static ADCdata readAllchannel(ADCdata& myadcdata);
            static void ADCoff();
            static void ADCon();
            static bool ADCtest();
        private:
            void error(int err);
            int adcRead(int channel);  
            float ADCcalib(int reading, int chloval, int chhival);
    };
    
    class espADCcfg {
            public:
                espADCcfg();
                ~espADCcfg();
                bool loadConfiguration(const char *cfgFilename, ADCconfig &myadcconfig);
                void loadConfiguration();
                bool saveConfiguration(const char *cfgFilename, const ADCconfig &myadcconfig) ;
                void saveConfiguration();
                void printFile(const char *cfgFilename);
                void printFile();
                void printCFG();
                
            private:
                
        };
#endif