/*
 Get local timestamp and date
*/

#include <NTPClient.h>
#include <WiFiUdp.h>
////#include <Chrono.h>

////Chrono myChrono(Chrono::SECONDS);


#include "espTime.h"
#include "getCfg.h"

// Define NTP Client to get time
WiFiUDP ntpUDP;
  // You can specify the time server pool and the offset (in seconds, can be
  // changed later with setTimeOffset() ). Additionaly you can specify the
  // update interval (in milliseconds, can be changed using setUpdateInterval() ).
  // NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
NTPClient timeClient(ntpUDP, "pool.ntp.org");

espTime::espTime() { //Class constructor

};
espTime::~espTime() { //Class destructor

};
void espTime::setup() {
  
// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  // timeClient.setTimeOffset(-3600*6); //for CDT (-6) or CST (-5) -- UTC/GMT = 0  UTC is time standrd while GMT is timezone
}

String espTime::getCurTimestamp () {
  // initialize NTPClient
  espTime::setup();
  //Update time
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime();
  int currentHour = timeClient.getHours();  
  int currentMinute = timeClient.getMinutes();     
  int currentSecond = timeClient.getSeconds();
  
  String curTimeStamp = String(currentHour) + ":" + String(currentMinute) +  ":" + String(currentSecond);

  return curTimeStamp;

  }

String espTime::getCurDate () {
  // initialize NTPClient
  espTime::setup();
  //Update time
  timeClient.update();
  
  unsigned long epochTime = timeClient.getEpochTime();

  String formattedTime = timeClient.getFormattedTime();

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;  
  int currentMonth = ptm->tm_mon+1;  
  int currentYear = ptm->tm_year+1900;

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  
  return currentDate;
}

// Set time via NTP, as required for x.509 validation
void espTime::setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC

  Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
  }

  Serial.println(F(""));
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}