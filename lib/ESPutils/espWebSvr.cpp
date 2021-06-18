/*
  @file espWebSvr.cpp


*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

#if defined(ESP8266)  
  #include <ESP8266WebServer.h>  
#elif defined(ESP32)  
  #include <WebServer.h>
#else
  #error Invalid platform
#endif 

// #include <Ticker.h>

#include "index.html"  //include the index.html page
#include "config.h"
#include "getCfg.h"
#include "espWebSvr.h"
#include "espTime.h"
#include "espRelay.h"

//Initialize Webserver
ESP8266WebServer server(wsport);

espSwitch myswitch;
espTime mytime;
espRelay myrelay;

SwitchData myswitchdata;
// progStats myprogstats;

espWebSrv::espWebSrv() { //Class constructor
};
espWebSrv::~espWebSrv() { //Class destructor
};

espSwitch::espSwitch() { //Class constructor
};
espSwitch::~espSwitch() { //Class destructor
};

void espWebSrv::setupWebSvr(const int port) {
  // espWebSrv::initWebSvr(port);
  server.begin();
  Serial.printf("starting HTTP Serer on port: %i, %i \n", wsport, port);
  espWebSrv::restRouter();
};

void espWebSrv::restRouter(){
   server.on("/", HTTP_GET, espWebSrv::handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  
  server.on("/Switch-ON", HTTP_GET, espSwitch::handleSwitchON);   
  server.on("/Switch-OFF", HTTP_GET, espSwitch::handleSwitchOFF);
  server.on("/"+config.devicecfg.name, HTTP_GET, espSwitch::handleSwitchOps);   //Tuen switch ON or OFF based on Parameters

  server.on("/genericArgs", HTTP_GET, espSwitch::handleGenericArgs);    //Associate the handler function to the path
  server.on("/specificArgs", HTTP_GET, espSwitch::handleSpecificArg);   //Associate the handler function to the path
    
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

};

void espWebSrv::initWebSvr(const int port) {
  // //Initialize Webserver
  // ::ESP8266WebServer server(port);
};

void espWebSrv::WSStatus() {

};

int espWebSrv::getWSPort() {
  Serial.printf("Ws Port: %i \n", config.websvrcfg.port );
  return config.websvrcfg.port;
};


void espWebSrv::handleRoot() {
  String s = String(index_html); //Read HTML contents
  server.send(200, "text/html", s); 
  Serial.println("Webpage request received");
  //server.send(200, "text/html", "get Temperature \n get humidity \n");
};

void espWebSrv::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  };
  server.send(404, "text/plain", message);  
};

void espWebSrv::handleClient(){
  server.handleClient();
};

void espSwitch::handleSwitchOFF() {  
  bool SW_State = myrelay.relayOpen();
  server.send(200, "text/plain", String(SW_State));
};

void espSwitch::handleSwitchON() {  
  bool SW_State = myrelay.relayClose();
  server.send(200, "text/plain", String(SW_State));
    
};

void espSwitch::handleGenericArgs() {//Handler

String message = "Number of args received:";
message += server.args();            //Get number of parameters
message += "\n";                            //Add a new line

for (int i = 0; i < server.args(); i++) {

message += "Arg nº" + (String)i + " –> ";   //Include the current iteration value
message += server.argName(i) + ": ";     //Get the name of the parameter
message += server.arg(i) + "\n";              //Get the value of the parameter

} 

server.send(200, "text/plain", message);       //Response to the HTTP request

}

void espSwitch::handleSpecificArg() {//Handler
  String message = "";

  if (server.arg("Switch")== ""){     //Parameter not found

  message = "Switch Operation Argument not found";

  }else{     //Parameter found

  message = "Switch Argument = ";
  message += server.arg("Switch");     //Gets the value of the query parameter

  }

  server.send(200, "text/plain", message);          //Returns the HTTP response
}

void espSwitch::handleSwitchOps() {//Handler
  String message = "";

  if (server.arg("Switch")== ""){     //Parameter not found

  message = "Switch Operation Argument not found";

  }else{     //Parameter found

    // message = "Switch Argument = ";
    // message += server.arg("Switch");     //Gets the value of the query parameter

    String op = server.arg("Switch");
    bool SW_State;

    if (op == "ON") {
      bool SW_State = myrelay.relayClose(); //Turn Switch ON
      message = SW_State;
    } else if (op == "OFF") {
      bool SW_State = myrelay.relayOpen(); //Turn Switch OFF
      message = SW_State;
    } else  {
      message = "Only ON and OFF operations allowed";
    };
  // Does not work as in C++ switch can only use integers, not string
  //   switch("op") {
  //     case "ON" :
  //        SW_State = myrelay.relayClose(); //Turn Switch ON
  //        message = SW_State;
  //        break;      
  //     case "OFF" :
  //        SW_State = myrelay.relayOpen(); //Turn Switch OFF
  //        message = SW_State;
  //        break;
  //     default :
  //        message = "Only ON and OFF operations allowed";
  //   }
  }

  server.send(200, "text/plain", message);          //Returns the HTTP response
};