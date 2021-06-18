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
            static String getCurTimestamp(); 
            static String getCurDate();
            static void setClock();   
            // static void espTime::showTime(); 
            static void showTime(tm localTime);
        private:
            static void setup();
            static bool getNTPtime(int sec);
    };
#endif