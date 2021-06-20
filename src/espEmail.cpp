#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <base64.h>

#include "espEmail.h"
#include "config.h"

espEmail espemail;

String        error_message = "";
String        Response;


String To, Subject, Message, Login_base64, Passwrd_base64;

WiFiClientSecure client;

espEmail::espEmail() { //Class constructor
};
espEmail::~espEmail() { //Class distructor
};

void espEmail::setup() {
  // Enter your own data for who you are sending the emails to 
  // and also any subject or message. If you have multiple sensor
  // setups, you can tailor each email to the particular sensor
  To      = "ovace.home@gmail.com"; 
  From    = "ovace.home@gmail.com"; // Gmail generally prefers this to be same as Senders_Login and may substitute
  Subject = "Water " + config.devicecfg.location + " at " + config.devicecfg.name + " Triggered";
  Message = "Water " + config.devicecfg.location + " at " + config.devicecfg.name + " Triggered";
};

void espEmail::SendMail(String From, String To, String Subject, String Message) {
  
  if (!client.connect(SMTP_SERVER, SMTP_PORT)) {
    error_message = "SMTP could not connect to the mail server";
    return;
  }
  if (espemail.ErrorWhileWaitingForSMTP_Response("220", 500)) {
    error_message = "SMTP Connection Error";
    return;
  }
  client.println("HELO server");
  if (espemail.ErrorWhileWaitingForSMTP_Response("250", 500)) {
    error_message = "SMTP Identification error";
    return;
  }
  client.println("AUTH LOGIN");
  espemail.WaitSMTPResponse(Response, 500);
  client.println(base64::encode(Senders_Login));
  
  client.println(base64::encode(Senders_Password));

  if (espemail.ErrorWhileWaitingForSMTP_Response("235", 500)) {
    error_message = "SMTP Authorisation error";
    return;
  }

  // Note that mailFrom and recipient (To) do not support
  // multiple email addresses in the string. The sending of the
  // message will fail.
  
  String mailFrom = "MAIL FROM: <" + From + '>';
  client.println(mailFrom);
  espemail.WaitSMTPResponse(Response, 500);
  
  String recipient = "RCPT TO: <" + To + '>';
  client.println(recipient);
  espemail.WaitSMTPResponse(Response, 500);
  client.println("DATA");
  
  if (espemail.ErrorWhileWaitingForSMTP_Response("354", 500)) {
    error_message = "SMTP DATA error";
    return;
  }
  
  client.println("From: <" + String(From) + '>');
  client.println("To: <" + String(To) + '>');
  client.print("Subject: ");
  client.println(String(Subject));
  client.println("Mime-Version: 1.0");
  client.println("Content-Type: text/html; charset=\"UTF-8\"");
  client.println("Content-Transfer-Encoding: 7bit");
  client.println();
  
  String body = "<!DOCTYPE html><html lang=\"en\">" + Message + "</html>";
  client.println(body);
  client.println(".");
  
  if (espemail.ErrorWhileWaitingForSMTP_Response("250", 1000)) {
    error_message = "SMTP Message error";
    return;
  }
  
  client.println("QUIT");
  if (espemail.ErrorWhileWaitingForSMTP_Response("221", 1000)) {
    error_message = "SMTP QUIT error";
    return;
  }
  
  client.stop();
}

bool espEmail::ErrorWhileWaitingForSMTP_Response(String Error_Code, int TimeOut) {
  int timer = millis();
  
  while (!client.available()) {
    
    if (millis() > (timer + TimeOut)) {
      error_message = "SMTP Timeout";
      return true;
    }
  }
  
  Response = client.readStringUntil('\n');
  
  if (Response.indexOf(Error_Code) == -1) return true;
  return false;
}

bool espEmail::WaitSMTPResponse(String Error_Code, int TimeOut) {
  int timer = millis();
  
  while (!client.available()) {
    
    if (millis() > (timer + TimeOut)) {
      error_message = "SMTP Timeout";
      return false;
    }
  }
  Response = client.readStringUntil('\n');
  
  if (Response.indexOf(Error_Code) == -1) return false;
  return true;
}
