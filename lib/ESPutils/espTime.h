#ifndef _ESP_Time_H
#define _ESP_Time_H

    #include <NTPClient.h>
    #include <WiFiUdp.h>
    #include "config.h"

    static String curTimestamp;
    static String curDate;    

    class espTime {
        public:
            espTime();
            ~espTime();
            static String getCurTimestamp(); 
            static String getCurDate();
            static void setClock();           
                        

        private:
            static void setup();
    };

    
       
#endif