#ifndef _ESP_OTA__H
#define _ESP_OTA__H


    class espOTA {
        public:
            espOTA();
            //~espOTA();
            static void setup();
            static void OTA_pull();
            static void OTA_push();
            static void OTA_push_handle();             

        private:            
            static void update_started();
            static void update_finished();
            static void update_progress(int cur, int total);
            static void update_error(int err);
            static String MACADDR2MACID(String addr);
    };

    
       
#endif