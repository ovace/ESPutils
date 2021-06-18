#include <Arduino.h>
#include <string>
#include <ArduinoJson.h>
#include <LittleFS.h>
//#include <FS.h>


/* 
#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif 
*/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
  #include <WiFiClient.h>
  #include <ESP8266mDNS.h>

  /**
  * ESP8266 Pin Connections
  * 
  * GPIO0/SPI-CS2/_FLASH_ -                           Pull low for Flash download by UART
  * GPIO1/TXD0/SPI-CS1 -                              _LED_
  * GPIO2/TXD1/I2C-SDA/I2SO-WS -
  * GPIO3/RXD0/I2SO-DATA - 
  * GPIO4 -
  * GPIO5/IR-Rx -
  * GPIO6/SPI-CLK -                                  Flash CLK
  * GPIO7/SPI-MISO -                                 Flash DI
  * GPIO8/SPI-MOSI/RXD1 -                            Flash DO
  * GPIO9/SPI-HD -                                   Flash _HD_ 
  * GPIO10/SPI-WP -                                  Flash _WP_
  * GPIO11/SPI-CS0 -                                 Flash _CS_
  * GPIO12/MTDI/HSPI-MISO/I2SI-DATA/IR-Tx -
  * GPIO13/MTCK/CTS0/RXD2/HSPI-MOSI/I2S-BCK -
  * GPIO14/MTMS/HSPI-CLK/I2C-SCL/I2SI_WS -
  * GPIO15/MTDO/RTS0/TXD2/HSPI-CS/SD-BOOT/I2SO-BCK - Pull low for Flash boot
  * GPIO16/WAKE -
  * ADC -
  * EN -
  * RST -
  * GND -
  * VCC -
  */

  /**
  * ESP-01 GPIO
    * GPIO0 -- 0
    * GPIO2 -- 2
    * TX    -- 1
    * RX    -- 3 
  */

#elif defined(ESP32)
  #include <WiFi.h>
  #include <mDNS.h>
  #include <WebServer.h>
#else
  #error Invalid platform
#endif 

#include "config.h"
#include "getCfg.h"
#include "espOTA.h"
#include "espTime.h"

#include "espEmail.h"
#include "espMQTT-WiFi.h"
#include "espRelay.h"
#include "espWebSvr.h"
#include "WiFi-DHT-MQTT-WebSvr.h"



//json construct setup
struct SensorData {
  String date;
  String time;
  int bootno;
  int sendCount;
  long int upTime;
  float interval;
  float temp;
  float humid;
  String code_rel;
  float cfg_rel;
  float Vcc;
};
SensorData sensordata;

void saveSensorData(const SensorData& sensordata);
void sendData(SensorData& sensordata);


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
long previousMillis = millis();        //initialize previousMillis variable in milliseconds
long previousTm = previousMillis; 
long startTm = previousMillis; 
int sendCount = 0;

char dayStamp[11];
char timeStamp1[6];

espWiFi mywifi;
espMQTT mymqtt;
espWebSrv mywebsvr;
espTime myTime;
espDHT mydht;
espOTA myota;
espCFG mycfg;

void setup() {
  //  mycfg.loadConfiguration();  
  cfgBegin();
  ];