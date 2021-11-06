/*
    @file=espADC.cpp

    Provides ADC Readings for each channel
    add following to the setup() in your main.cpp

    espADC::setup();

    add following to the loop in you main.cpp

    espADC:readADCChannel(channel);

    @dependency=
*/

// #include <SPI.h>
// SPI mySpi;
#include <Adafruit_MCP3008.h>
Adafruit_MCP3008 adc;
#include "LittleFS.h" // LittleFS is declared
#include <ArduinoJson.h>
#include <string.h>

#include "espADC.h"
espADC myAdc;
espADCcfg myAdcCfg;
ADCconfig  myadccfg, *adcCfgP = &myadccfg;
// adcCfg = &myadccfg;

static const char *cfgFilename = "/espADCconfig.json";  

#define _DEBUG_ myadccfg.debug

/*Label	GPIO	Input	Output	Notes
D0	GPIO16	no interrupt	no PWM or I2C support	HIGH at boot
used to wake up from deep sleep
D1	GPIO5	OK	OK	often used as SCL (I2C)
D2	GPIO4	OK	OK	often used as SDA (I2C)
D3	GPIO0	pulled up	OK	connected to FLASH button, boot fails if pulled LOW
D4	GPIO2	pulled up	OK	HIGH at boot
connected to on-board LED, boot fails if pulled LOW
D5	GPIO14	OK	OK	SPI (SCLK)
D6	GPIO12	OK	OK	SPI (MISO)
D7	GPIO13	OK	OK	SPI (MOSI)
D8	GPIO15	pulled to GND	OK	SPI (CS)
Boot fails if pulled HIGH
RX	GPIO3	OK	RX pin	HIGH at boot
TX	GPIO1	TX pin	OK	HIGH at boot
debug output at boot, boot fails if pulled LOW
A0	ADC0	Analog Input	X	
*/
//define pin connections
#define MISO_PIN myadccfg.spicfg.MISOpin //MISO -- D7
#define MOSI_PIN myadccfg.spicfg.MOSIpin //MOSI -- D6
#define SCLK_PIN myadccfg.spicfg.SCLKpin // SCLK -- D4
#define CS_PIN myadccfg.spicfg.CSpin // CS -- D8

/* MCP3008   ESP8266
V_DD      3v3
V_REF     3v3
AGND      GND
CLK       GPIO14 (D5)
D_OUT     GPIO12 (D6)
D_IN      GPIO13 (D7)
CS        GPIO15 (D8)
DGND      GND
 */
//MCP3008 adc(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
// MCP3008 adc;
// Adafruit_MCP3008 adc;

espADC::espADC() { //Class constructor
};
espADC::~espADC() { //Class destructor
};
bool espADC::setup(){  
  // read configuraton file
  myAdcCfg.loadConfiguration();

  if (_DEBUG_) {
    Serial.printf("Chip ID: %s", String(ESP.getChipId()).c_str()); Serial.println();
    Serial.printf("Device Vcc: %s", String(ESP.getVcc()).c_str()); Serial.println();

    Serial.print("Configuration JSON File: ");Serial.println(cfgFilename);
    myAdcCfg.printFile();

    Serial.println("Configuration: ");
    myAdcCfg.printCFG();

  } 
  
// Initialize for hardware SPI

// /*!
//  *    @brief  Initialize for hardware SPI
//  *    @param  cs
//  *            number of CSPIN (Chip Select)
//  *    @param  theSPI
//  *            optional SPI object
//  *    @return true if process is successful
//  */
// bool Adafruit_MCP3008::begin(uint8_t cs, SPIClass *theSPI) 

  // /*!
  // *    @brief  Initialize for software SPI
  // *    @param  sck
  // *            number of pin used for SCK (Serial Clock)
  // *    @param  mosi
  // *            number of pin used for MOSI (Master Out Slave In)
  // *    @param  miso
  // *            number of pin used for MISO (Master In Slave Out)
  // *    @param  cs
  // *            number of pin used for CS (Chip Select)
  // *    @return true if process is successful
  // */
  // bool Adafruit_MCP3008::begin(uint8_t sck, uint8_t mosi, uint8_t miso, uint8_t cs) {
  // Use the default SPI hardware interface.
  // bool ADCReady = adc.begin(CS_PIN);
  // bool ADCReady = adc.begin(SCLK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  // bool ADCReady = adc.begin(14, 13, 12, 15);
  // Or use custom pins to use a software SPI interface.
  // adc.begin(SS, MOSI, MISO, SCK);
  // adc.begin(CS_PIN,  MOSI_PIN, MISO_PIN, SCLK_PIN);

  // Or use custom pins to use a software SPI interface.
  // adc.begin(SS, MOSI, MISO, SCK);

  // Initializing serial port for SPI comms
  int _spibaud = myadccfg.adccfg.Bauds;
  if(_spibaud <= 0){ _spibaud = 74880;};

  Serial.begin(_spibaud);

  // pinMode(CS_PIN, OUTPUT);
  // myAdc.ADCoff();  
  Serial.println(F("Enabling ADC"));
  myAdc.ADCon();
  delay(20);
  if (_DEBUG_) {
    Serial.println(SCLK_PIN);
    Serial.println(MISO_PIN);
    Serial.println(MOSI_PIN);
    Serial.println(CS_PIN);
  }
   bool ADCReady;
  // if (mySpi.pins(SCLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN)) {
  // mySpi.begin();
  // mySpi.setBitOrder(MSBFIRST);
  // mySpi.setDataMode(SPI_MODE0);
  // mySpi.setFrequency(10000); // 1 MHz -- remove line when running on Arduino UNO

  // (sck, mosi, miso, cs);
  adc.begin(SCLK_PIN, MOSI_PIN, MISO_PIN, CS_PIN);
  ADCReady = true;

  Serial.println(F("ADC Ready"));
  // }
  return ADCReady;
}
ADCdata espADC::readAllchannel(ADCdata &myadcdata) {
for (int chan=0; chan<myadccfg.adccfg.numChannels; chan++) {
  if (_DEBUG_) {
  Serial.print(myAdc.adcRead(chan)); Serial.print(",\t");
  }
  myadcdata.ADC[chan] = myAdc.adcRead(chan);
  }
  return myadcdata;  
}
int espADC::readChannel(uint8_t channel) {
  int chan_val = myAdc.adcRead(channel);
  if (_DEBUG_) {
    Serial.printf("Channel %i: %i",channel,chan_val); Serial.println();
  }
  return chan_val;   
}
void espADC::error(int err) {
  if (!myAdc.ADCtest()) {
    Serial.printf("ADC read error:  %d\n", err);
  }
}
int espADC::adcRead(int channel) {
  if ((channel > myadccfg.adccfg.numChannels) || (channel < 0)) {
    Serial.println(F("ADC Channel Read error"));
    return -1;
  }
  myAdc.ADCon();

  // mySpi.transfer(1);
  // uint8_t r1 = mySpi.transfer((channel + 8) << 4);
  // uint8_t r2 = mySpi.transfer(0);

  int chan_val = adc.readADC(channel);

  myAdc.ADCoff();

  int chlo = myadccfg.calib.CHlo[channel];
  int chhi = myadccfg.calib.CHhi[channel];

  if (_DEBUG_) {
    Serial.print(F("CH Lo: ")); Serial.println(chlo);
    Serial.print(F("CH Hi: ")); Serial.println(chhi);
  }

  // return ((r1 & 3) << 8) + r2;
  return map(chan_val, chlo, chhi, 0, 100);
}
float espADC::ADCcalib(int reading, int chloval, int chhival) {
    return 0;
}
void espADC::ADCoff() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  // mySpi.end();
}
void espADC::ADCon() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, LOW);
}
bool espADC::ADCtest() {
  // myAdc.readAllchannel();
  return 1;
}

espADCcfg::espADCcfg() { //Class constructor
};
espADCcfg::~espADCcfg() { //Class destructor
};
bool espADCcfg::loadConfiguration(const char *filename, ADCconfig &myadccfg){
//mounts file system.
  LittleFS.begin();

  FSInfo fs_info;
  LittleFS.info(fs_info);
    Serial.print(F("Total Bytes: "));Serial.println( fs_info.totalBytes);
    Serial.print(F("used Bytes: "));Serial.println(fs_info.usedBytes);
    Serial.print(F("Block Size: "));Serial.println(fs_info.blockSize);
    Serial.print(F("Page Size: "));Serial.println(fs_info.pageSize);
    Serial.print(F("max open files: "));Serial.println(fs_info.maxOpenFiles);
    Serial.print(F("max path lengt: "));Serial.println(fs_info.maxPathLength);

  // Open file for reading
  if (LittleFS.exists(filename)) {

    File file = LittleFS.open(filename, "r");
    Serial.print(F("Opening myadccfg file: ")); Serial.println(filename);
    if (!file) {
      Serial.print(F("Failed to read file")); Serial.println(filename); 
      return 0;
      };

    const int capacity =   3*JSON_OBJECT_SIZE(2) + 
                           JSON_OBJECT_SIZE(4) +
                           JSON_OBJECT_SIZE(16) +
                           200;
    Serial.print("JsonBufferCapacity: "); Serial.println(capacity);
    StaticJsonDocument<capacity> doc;
    // StaticJsonDocument<300> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error) {  
      Serial.print(F("Failed to read file, using default configuration"));
      Serial.println(error.f_str());
      return 0;
    };
    // Copy values from the JsonDocument to the myadccfg

    //Release
    myadccfg.cfg_rel = doc["release"] | 99.99;
    myadccfg.debug = doc["debug"] | false;

    //SPI
    JsonObject SPI = doc["SPI"];
    myadccfg.spicfg.SCLKpin = SPI["SCLK"]; // 14
    myadccfg.spicfg.MISOpin = SPI["MISO"]; // 12
    myadccfg.spicfg.MOSIpin = SPI["MOSI"]; // 13
    myadccfg.spicfg.CSpin = SPI["CS"]; // 15

    //ADC
    JsonObject ADC = doc["ADC"];
    myadccfg.adccfg.Bauds = ADC["bauds"] |  74880;
    myadccfg.adccfg.numChannels = ADC["numChannel"] |  74880;  

    //UART
    JsonObject UART = doc["UART"];
    myadccfg.uartcfg.TXpin = UART["TX"]; // 3
    myadccfg.uartcfg.RXpin = UART["RX"];// 1

    //Calibration
    JsonArray Calib = doc["ADCcalibration"];
    int arraySize =   Calib.size();

    if (_DEBUG_) {
        Serial.print(F("arraySize: ")); Serial.println(arraySize);
      }

    JsonArray ADCcalibLo = Calib[0];
    int arrayLoSize =   ADCcalibLo.size();
    for (int j = 0; j < arrayLoSize; j++) {      
      adcCfgP->calib.CHlo[j] = ADCcalibLo[j]; 

      if (_DEBUG_) {
        Serial.print(F("CHLo-"));Serial.print(j);Serial.print(F(": "));Serial.println(adcCfgP->calib.CHlo[j]);
      }
    }

    JsonArray ADCcalibHi = Calib[1];
    int arrayHiSize =   ADCcalibHi.size();
    for (int j = 0; j < arrayHiSize; j++) {      
      adcCfgP->calib.CHhi[j] = ADCcalibHi[j]; 

      if (_DEBUG_) {
        Serial.print(F("CHHi-"));Serial.print(j);Serial.print(F(": "));Serial.println(adcCfgP->calib.CHhi[j]);
      }
    }

    
    
    file.close(); // Close the file (Curiously, File's destructor doesn't close the file)
  } else {
    Serial.print(F("File not found: ")); Serial.println(filename); 
    return 0;
  };  
  Serial.println("done processing JSON");  

  LittleFS.end(); //unmounts file system.
  return 1;
};
void espADCcfg::loadConfiguration(){
  espADCcfg::loadConfiguration(cfgFilename, myadccfg);
};
bool espADCcfg::saveConfiguration(const char *filename, const ADCconfig &myadccfg) {
  //mounts file system.
  LittleFS.begin();
  // Delete existing file, otherwise the configuration is appended to the file
  LittleFS.remove(filename);

  // Open file for writing
  File file = LittleFS.open(filename, "r+");
  if (!file) {
    Serial.print(F("Failed to create file: ")); Serial.println(filename);
    return 0;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  // doc["hostname"] = myadccfg.wirelesscfg.hostname;
  // doc["port"] = myadccfg.wirelesscfg.port;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file")); Serial.println(filename);
    return 0;
  }

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();
  return 1;
};
void espADCcfg::saveConfiguration(){
  espADCcfg::saveConfiguration(cfgFilename, myadccfg);
};
void espADCcfg::printFile(const char *filename){
  //mounts file system.
  LittleFS.begin();

  FSInfo fs_info;
  LittleFS.info(fs_info);

  Serial.print(F("Total Bytes: "));Serial.println( fs_info.totalBytes);
  Serial.print(F("used Bytes: "));Serial.println(fs_info.usedBytes);
  Serial.print(F("Block Size: "));Serial.println(fs_info.blockSize);
  Serial.print(F("Page Size: "));Serial.println(fs_info.pageSize);
  Serial.print(F("max open files: "));Serial.println(fs_info.maxOpenFiles);
  Serial.print(F("max path lengt: "));Serial.println(fs_info.maxPathLength);

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.print(F("Failed to read file: "));Serial.println(filename);
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();

  //unmounts file system.
  LittleFS.end();
};
void espADCcfg::printFile(){
  espADCcfg::printFile(cfgFilename);
};
void espADCcfg::printCFG(){
  Serial.printf("Release: %f ",myadccfg.cfg_rel);  Serial.println();
  Serial.printf("Debug mode: %s ", myadccfg.debug? "true"  : "false"); Serial.println();
  
  Serial.println(F("SPI myadccfg"));
  Serial.println(myadccfg.spicfg.MISOpin);
  Serial.println(myadccfg.spicfg.MOSIpin);
  Serial.println(myadccfg.spicfg.SCLKpin);
  Serial.println(myadccfg.spicfg.CSpin);

  Serial.println(F("UART myadccfg"));
  Serial.println(myadccfg.uartcfg.TXpin);
  Serial.println(myadccfg.uartcfg.RXpin);
  
  Serial.println(F("ADC myadccfg"));
  Serial.println(myadccfg.adccfg.Bauds);
  Serial.println(myadccfg.adccfg.numChannels);
  Serial.println(myadccfg.adccfg.Type);

  Serial.println(F("Calibration myadccfg"));
  int arrayLoSize =   sizeof(myadccfg.calib.CHlo)/sizeof(myadccfg.calib.CHlo[0]);
  
  for (int j = 0; j < arrayLoSize; j++) {      
      Serial.print(F("CH"));Serial.print(j);Serial.print(F("Lo: "));Serial.println(myadccfg.calib.CHlo[j]); 
  }

  int arrayHiSize =   sizeof(myadccfg.calib.CHhi)/sizeof(myadccfg.calib.CHhi[0]);
  for (int j = 0; j < arrayHiSize; j++) {      
      Serial.print(F("CH"));Serial.print(j);Serial.print(F("Lo: "));Serial.println(myadccfg.calib.CHhi[j]); 
  }

};


