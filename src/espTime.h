#ifndef _ESP_Time_H
#define _ESP_Time_H

    #include <Arduino.h>
    #include "config.h"

    static String curTimestamp;
    static String curDate;    

    class espTime {
        public:
            espTime();
            ~espTime();
            static void NTPstart();
            static String getCurTimestamp(); 
            static String getCurDate();
            static void setClock();   
            static void showTime();
            static void showTime(tm *localTime);
            static void getEpoch();
            static void getTimeData();
            static void example();
        private:            
            boolean getNTP();
            bool getNTPtime(int sec);
    };
#endif