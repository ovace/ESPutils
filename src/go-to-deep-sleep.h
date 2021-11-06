#pragma once

#ifndef __esp2Sleep_H
#define __esp2Sleep_H

class go2Sleep {
    public:
        go2Sleep();
        ~go2Sleep();  
        static void setup();
        static void get2SleepCfg();          
        static void goToDeepSleep(int timeinsec);
        static void goToDeepSleepFiveMinutes();
    private:

};      
#endif /* __esp2Sleep_H */