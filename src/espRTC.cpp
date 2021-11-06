/*

    @file=espRTC.cpp

    get infroamtion from RTC
    add following to the setup() in your main.cpp

    espRTC::setup();

    add following to the loop in you main.cpp

    espRTC:readRTC();

    @dependency=

*/

#if defined(ESP8266)
    #include <ESP8266WiFi.h>  // default from Espressif  
#elif defined(ESP32)
    #include <WiFi.h>         // defaults from Espressif  
#endif

#include <include/WiFiState.h> // WiFiState structure details
  WiFiState state; // Size of this is 152 bytes

#include "espRTC.h"
espRTC myRTC;



// We make a structure to store connection information
// The ESP8266 RTC memory is arranged into blocks of 4 bytes. The access methods read and write 4 bytes at a time,
// so the RTC data structure should be padded to a 4-byte multiple.
struct {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t ap_mac[6];// 6 bytes, 11 in total
  uint8_t padding;  // 1 byte,  12 in total
} rtcData;

espRTC::espRTC() { //Class constructor
};
espRTC::~espRTC() { //Class destructor
};

void espRTC::setup() {
  #ifndef RTC_USER_DATA_SLOT_WIFI_STATE
  #define RTC_USER_DATA_SLOT_WIFI_STATE 33u // Data stored in first 32 blocks overwritten by OTA updates
#endif

};

void espRTC::write() {

// Write current connection info back to RTC
  rtcData.channel = WiFi.channel();
  memcpy( rtcData.ap_mac, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
  rtcData.crc32 = myRTC.calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
  ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );
 
}

bool espRTC::read() {
// Try to read WiFi settings from RTC memory
  bool rtcValid = false;
  if ( ESP.rtcUserMemoryRead( 0, (uint32_t*)&rtcData, sizeof( rtcData ) ) ) {
    // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = myRTC.calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    if ( crc == rtcData.crc32 ) {
      rtcValid = true;
    }    
  }
  return rtcValid;
}

// the CRC routine
uint32_t espRTC::calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while ( length-- ) {
    uint8_t c = *data++;
    for ( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if ( c & i ) {
        bit = !bit;
      }

      crc <<= 1;
      if ( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }

  return crc;
}