/*
 Get local timestamp and date
*/

#include <Arduino.h>
#include <time.h>           // time() ctime()
#include <string.h>

#include "espTime.h"
#include "getCfg.h"

espTime esptime;

/* Globals */
struct tm;
time_t now;                 // this is the epoch
tm timeinfo;                // the structure timeinfo holds time information in a more convient way
long unsigned lastNTPtime;
unsigned long lastEntryTime;



// // Define NTP Client to get time
// WiFiUDP ntpUDP;
//   // You can specify the time server pool and the offset (in seconds, can be
//   // changed later with setTimeOffset() ). Additionaly you can specify the
//   // update interval (in milliseconds, can be changed using setUpdateInterval() ).
//   // NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
// NTPClient timeClient(ntpUDP, "pool.ntp.org");

/* Configuration of NTP */
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"           
// #define MY_TZ 	"CST6CDT,M3.2.0,M11.1.0"  // see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
#define MY_TZ 	"UTC0"  // UTC Etc/Universal	

espTime::espTime() { //Class constructor
};
espTime::~espTime() { //Class destructor
};

void espTime::setup() {

  configTime(MY_TZ, NTP_SERVER1, NTP_SERVER2); // --> Here is the IMPORTANT ONE LINER needed in your sketch!  


  // configTime(0, 0, NTP_SERVER1, NTP_SERVER2); // --> Here is the IMPORTANT ONE LINER needed in your sketch!  
  
  // setenv("TZ", MY_TZ, 1);  //not avalable in windows
  // putenv("TZ=" MY_TZ);  // if not included, localtime = gmtime

  // Serial.println(esptime.getNTPtime(10));
  if (esptime.getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
    Serial.println("Time not set");
    ESP.restart();
  };
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

}

String espTime::getCurTimestamp () {
  // initialize NTPClient
  esptime.setup();
  //Update time  
  time(&now);                       // read the current time
  localtime_r(&now, &timeinfo);     // update the structure tm with the current time

  int currentHour = timeinfo.tm_hour;  
  int currentMinute = timeinfo.tm_min;     
  int currentSecond = timeinfo.tm_sec;
  
  String curTimeStamp = String(currentHour) + ":" + String(currentMinute) +  ":" + String(currentSecond);

  return curTimeStamp;

  }

String espTime::getCurDate () {
  // initialize NTPClient
  esptime.setup();
 
  time(&now);                       // read the current time
  localtime_r(&now, &timeinfo);     // update the structure tm with the current time

  //Get a time structure
  int monthDay = timeinfo.tm_mday;  
  int currentMonth = timeinfo.tm_mon+1;  
  int currentYear = timeinfo.tm_year+1900;

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  
  return currentDate;
}

/* 
void espTime::showTime() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
  Serial.print("year:");
  Serial.print(tm.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tm.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tm.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(tm.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(tm.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(tm.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(tm.tm_wday);         // days since Sunday 0-6
  if (tm.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
} 
*/

 // Shorter way of displaying the time
void espTime::showTime(tm localTime) {
  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d, day %d, %s time\n",
    localTime.tm_year + 1900,
    localTime.tm_mon + 1,
    localTime.tm_mday,
    localTime.tm_hour,
    localTime.tm_min,
    localTime.tm_sec,
    (localTime.tm_wday > 0 ? localTime.tm_wday : 7 ),
    (localTime.tm_isdst == 1 ? "summer" : "standard")
  );
}

bool espTime::getNTPtime(int sec) {
  
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      
      if (_DEBUG_){
        Serial.println(now);
        Serial.println(millis());
        Serial.println(start);
        Serial.printf("sec %i \n",sec);
        Serial.println(timeinfo.tm_year);
        Serial.println(millis() - start);
        Serial.println(2016 - 1900);
      };
      
      Serial.print(".");
      delay(10);
    } while (((millis() - start) <= (1000 * sec))
       && (timeinfo.tm_year < (2016 - 1900))
      );
    if (timeinfo.tm_year <= (2016 - 1900)) {
      return false;  // the NTP call was not successful
    };
    // time(&now);
    // localtime_r(&now, &timeinfo);
    Serial.print("now ");  Serial.println(now);
    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
    Serial.println(time_output);
    Serial.println();
  
  return true;
}

