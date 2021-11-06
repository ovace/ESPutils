#ifndef _ESP_OTA__H
#define _ESP_OTA__H

struct OTAconfig {
    float cfg_rel;
    bool debug;
    struct OTAcfg {
        int port;
        String update_server;
        String url;
        int update_server_port;
        String secret;
        String user;
        String pswd;
    };
    OTAcfg otacfg;
};
    class espOTA {
        public:
            espOTA();
            ~espOTA();
            static void setup(String CUR_FW_VER);
            static void OTA_pull(String CUR_FW_VER);
            static void OTA_push();
            static void OTA_push_handle();             

        private:            
            static void update_started();
            static void update_finished();
            static void update_progress(int cur, int total);
            static void update_error(int err);
            static String MACADDR2MACID(String addr);
    };
    class espOTAcfg {
            public:
                espOTAcfg();
                ~espOTAcfg();
                void loadConfiguration(const char *cfgFilename, OTAconfig &myotacfg);
                void loadConfiguration();
                void saveConfiguration(const char *cfgFilename, const OTAconfig &myotacfg) ;
                void saveConfiguration();
                void printFile(const char *cfgFilename);
                void printFile();
                void printCFG();
                
            private:
                
        };
    
       
#endif