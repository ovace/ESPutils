#ifndef _ESP_RTC_H
#define _ESP_RTC_H

    #include <Arduino.h>
    // #include "config.h"

    
    class espRTC {
        public:
            espRTC();
            ~espRTC();
            static void setup();
            static bool read();
            static void write();
            
        private:            
            uint32_t calculateCRC32( const uint8_t *data, size_t length );
    };
#endif