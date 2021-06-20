/*
 Get local timestamp and date
*/

#include <Arduino.h>
#include <string.h>

//**Do not*** include other fancy Time libraries! Everything is on board from Espressif.
#if defined(ESP8266)
// #include <ESP8266WiFi.h>  // default from Espressif
  #include <WiFiUdp.h>      // default from Espressif 
  #include <TZ.h>           // default from Espressif
#elif defined(ESP32)
// #include <WiFi.h>         // defaults from Espressif
  #include <time.h>           // time() ctime()
#endif

#include "espTime.h"
#include "getCfg.h"

espTime esptime;

/* Globals */
// Variables for Time
struct tm;
static int lastSecond;
tm*        timeinfo;         //localtime returns a pointer to a tm structstatic int Second; // the structure timeinfo holds time information in a more convient way
time_t     Epoch;
time_t     now;             // this is the epoch

               
long unsigned lastNTPtime;
unsigned long lastEntryTime;

// maintain for ESP32
#define TZ            1                 // (utc+) TZ in hours
#define DST_MiN        60               // use 60mn for summer time in some countries
#define GMT_OFFSET_SEC 3600 * TZ        // Do not change here...
#define DAYLIGHT_OFFSET_SEC 60 * DST_MiN // Do not change here...
// maintain for ESP8266
/* Configuration of NTP */
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"           
// #define MY_TZ 	"CST6CDT,M3.2.0,M11.1.0"  // see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
#define MY_TZ 	"UTC0"  // UTC Etc/Universal	


int Second;
int Minute;
int Hour;
int Day;
int Month;
int Year;
int Weekday;
char DayName[12];
char MonthName[12];
char Time[10];
char Date[12];

//*** Buffers ***
char charbuff[80];    // Char buffer for many functions
//String text;        // String buffer for many functions

// // Define NTP Client to get time
// WiFiUDP ntpUDP;
//   // You can specify the time server pool and the offset (in seconds, can be
//   // changed later with setTimeOffset() ). Additionaly you can specify the
//   // update interval (in milliseconds, can be changed using setUpdateInterval() ).
//   // NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
// NTPClient timeClient(ntpUDP, "pool.ntp.org");



espTime::espTime() { //Class constructor
};
espTime::~espTime() { //Class destructor
};

void espTime::setup() {

    esptime.getNTP();

  // configTime(MY_TZ, NTP_SERVER1, NTP_SERVER2); // --> Here is the IMPORTANT ONE LINER needed in your sketch!  

  // configTime(0, 0, NTP_SERVER1, NTP_SERVER2); // --> Here is the IMPORTANT ONE LINER needed in your sketch!  
  
  // setenv("TZ", MY_TZ, 1);  //not avalable in windows
  // putenv("TZ=" MY_TZ);  // if not included, localtime = gmtime
  // TZset();

  // Serial.println(esptime.getNTPtime(10));
  // if (esptime.getNTPtime(10)) {  // wait up to 10sec to sync
  // } else {
  //   Serial.println("Time not set");
  //   ESP.restart();
  // };

  // showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

};

String espTime::getCurTimestamp () {
  // initialize NTPClient
  esptime.setup();
  //Update time  
  time(&now);                       // read the current time
  localtime_r(&now, timeinfo);     // update the structure tm with the current time

  int currentHour = timeinfo -> tm_hour;  
  int currentMinute = timeinfo -> tm_min;     
  int currentSecond = timeinfo -> tm_sec;
  
  String curTimeStamp = String(currentHour) + ":" + String(currentMinute) +  ":" + String(currentSecond);

  return curTimeStamp;

  }

String espTime::getCurDate () {
  // initialize NTPClient
  esptime.setup();
 
  time(&now);                       // read the current time
  localtime_r(&now, timeinfo);     // update the structure tm with the current time

  //Get a time structure
  int monthDay = timeinfo -> tm_mday;  
  int currentMonth = timeinfo -> tm_mon + 1;  
  int currentYear = timeinfo -> tm_year + 1900;

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
void espTime::showTime(tm *localTime) {
  Serial.printf(
    "%04d-%02d-%02d %02d:%02d:%02d, day %d, %s time\n",
    localTime -> tm_year + 1900,
    localTime -> tm_mon + 1,
    localTime -> tm_mday,
    localTime -> tm_hour,
    localTime -> tm_min,
    localTime -> tm_sec,
    (localTime -> tm_wday > 0 ? localTime -> tm_wday : 7 ),
    (localTime -> tm_isdst == 1 ? "summer" : "standard")
  );
}

bool espTime::getNTPtime(int sec) {
  
    uint32_t start = millis();
    char time_output[30];
    do {
      time(&now);
      localtime_r(&now, timeinfo);
      
      if (_DEBUG_){
        Serial.println("getNTPtime debug start");
        Serial.print("now ");  Serial.println(now);    
        strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
        Serial.print("timestamp "); Serial.println(time_output);
        Serial.print("millis "); Serial.println(millis());
        Serial.print("start "); Serial.println(start);
        Serial.printf("sec %i \n",sec);
        Serial.print("timeinfo.tm_year "); Serial.println(timeinfo -> tm_year);
        Serial.print("millis() - start ");Serial.println(millis() - start);
        Serial.println("2016 - 1900 ");Serial.println(2016 - 1900);
        Serial.println("getNTPtime debug End***");
      };
      
      Serial.print(">");
      delay(10);
    } while (((millis() - start) <= (1000 * sec))
       && (timeinfo -> tm_year < (2016 - 1900))
      );
    if (timeinfo -> tm_year <= (2016 - 1900)) {
      return false;  // the NTP call was not successful
    };
     
  return true;
}

void espTime::getNTP()
{
#if defined(ESP8266)
  configTime(MY_TZ, NTP_SERVER1, NTP_SERVER2);
#elif defined(ESP32)
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
#endif
  now = time(nullptr);
  Epoch = now;
};

void espTime::getEpoch()  // writes the Epoch (Numbers of seconds till 1.1.1970.
{
  now = time(nullptr);
  Epoch = now;

  if (_DEBUG_) {
    Serial.println("Start debug -> espTime::getEpoch");
    esptime.example();
    Serial.println("**Start debug -> espTime::getEpoch**");
  }; // end _DEBUG_
};

void espTime::getTimeData() // breaks down the Epoch into discrete values.
{
  timeinfo  = localtime(&now);  // cf: https://www.cplusplus.com/reference/ctime/localtime/
  Second    = timeinfo->tm_sec;
  Minute    = timeinfo->tm_min;
  Hour      = timeinfo->tm_hour;
  Weekday   = timeinfo->tm_wday + 1 ;
  Day       = timeinfo->tm_mday;
  Month     = timeinfo->tm_mon + 1;
  Year      = timeinfo->tm_year + 1900; //returns years since 1900
  strftime (DayName , 12, "%A", timeinfo); //cf: https://www.cplusplus.com/reference/ctime/strftime/
  strftime (MonthName, 12, "%B", timeinfo);
  strftime (Time,10, "%T", timeinfo);
  strftime (Date,12, "%d/%m/%Y", timeinfo);

  if (_DEBUG_) {
    Serial.println("Start debug -> espTime::getTimeData");
    esptime.example();
    Serial.println("**Start debug -> espTime::getTimeData**");
  }; // end _DEBUG_
};

void espTime::example()
{
  // esptime.getEpoch();            
  // esptime.getTimeData();         

  // Examples with Strings
  Serial.print(F("It is: "));   Serial.print( DayName );
  Serial.print(F("! Date is "));   Serial.print( Date );
  Serial.print(F(" and Time is "));   Serial.println( Time );

  // Examples with a character buffer (can be used for displays as well)
  sprintf(charbuff, "Now is %02d Hour, %02d Minutes and %02d Seconds. The Epoch is: %10lu" , Hour , Minute, Second, Epoch);
  Serial.println(charbuff);
  sprintf(charbuff, "Date is %s, %02d %s %04d ", DayName , Day , MonthName, Year);
  Serial.print(charbuff);
  Serial.println();

  // Examples with Posix expressions
  strftime (charbuff, 80, "Americans say it is %I:%M%p, but Europeans prefer using %T", timeinfo);
  Serial.println(charbuff);
  Serial.println();
  Serial.println();
  delay(1000);
};
