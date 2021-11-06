#ifndef _ESP_email_H
#define _ESP_email_H

    
    #include "config.h"
    #include <WiFiClientSecure.h>
    #include <base64.h>

    
    class espEmail {
    public:
        espEmail();
        //~myEmail();
        static void SendMail(String From, String To, String Subject, String Message);  

    private:
        static void setup();
        static bool ErrorWhileWaitingForSMTP_Response(String Error_Code, int TimeOut);
        static bool WaitSMTPResponse(String Error_Code, int TimeOut) ;

    };

    
       
#endif