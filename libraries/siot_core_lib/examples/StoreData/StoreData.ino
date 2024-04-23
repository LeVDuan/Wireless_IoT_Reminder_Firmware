//----------------------------------------------------------------------
//  IDE: Visual Studio Code + PlatformIO extension
//  Organization: School of Information and Communication Technogloy,
//                Ha Noi Univerversity of Science and Technology
//  Website: https://soict.hust.edu.vn
//  Example: Send data to the SIOT Center
//  Purpose: using siot core lib to send data from your IoT to the
//           correctsponding virtual device on SIOT Center
//  Using SIOT Center and virtual device:
//  1. Open the SIOT Center http://siot.soict.ai
//  2. Login with account sinno@soict.hust.edu.vn and password is 123456
//  3. On the left navigation menu, click Arduino Siot Client to see realtime data
//  Shortlink: http://siot.soict.ai/devices/siot-demo
//
//----------------------------------------------------------------------
#include <Arduino.h>
#if true
  #include <siot_core_lib.h>    // SIOT Core Lib - all packages or you could select each package manually.
#else
  #include <WiFiSelfEnroll.h>   // SIOT Core Lib - seft setup wifi network
  #include <siotclient.h>       // SIOT Core Lib - send data to siot api center
#endif


// Handler adhoc wifi station
WiFiSelfEnroll * MyWiFi = new WiFiSelfEnroll();

// SIoT Management Handler, send/receive data to/from SIOT API Center 
SIOTClient siotclient;

void setup() {
  //just for debugging
  Serial.begin(115200);
  // Make sure WiFi ssid/password is correct. Otherwise, raise the Adhoc AP Station with ssid = SOICT_CORE_BOARD and password =  12345678
  MyWiFi->setup();
  // Release the memory allocated for WiFi Station Handler after finishing his work
  delete  MyWiFi;
  MyWiFi = _NULL;
  // Declare the virtual device information which you have register at http://siot.soict.ai (free)
  siotclient.Checkin("siot-demo","eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiIwZDI1NWI5Ni05OWMyLTRmNTItOTg1NS0yZWRhNmI3MzYyMzAiLCJpYXQiOjE2ODcwNzQ4OTd9.SHRCTNASXYEPkYR-ZVadu1P19UPRTHQoHcZjzAoUaiM");
  // Send data to the attribute whose id is turnon-siot. Free to declare at siot.soict.ai as you need.
  siotclient.Send("turnon-siot","{\"value\":307}");
  // Anothor attribute
  Serial.print(F("HTTP Response code: "));  
  Serial.println(siotclient.Send("mysensor-fxcjl","{\"value\":15.2025}"));
}

void loop(){

}
