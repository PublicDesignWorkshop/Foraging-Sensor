/* 

 */

// Include required libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#include<stdlib.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and   MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

// Buffer for float to String conversion
// The conversion goes from a float value to a String with two numbers after the decimal.  That means a buffer of size 10 can accommodate a float value up to 999999.99 in order for the last entry to be \0
char buffer[10];

const unsigned long
dhcpTimeout     = 60L * 1000L, // Max time to wait for address from DHCP
connectTimeout  = 15L * 1000L, // Max time to wait for server connection
responseTimeout = 15L * 1000L; // Max time to wait for data from server

uint32_t t;

// THINGS THAT NEED CUSTOMIZATION///////////////////////////////////////////////////
// WiFi network (change with your settings!)
#define WLAN_SSID       "GTother"          // cannot be longer than 32 characters!
#define WLAN_PASS       "GeorgeP@1927" 
#define WLAN_SECURITY   WLAN_SEC_WPA2      // Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// define website and folder structure
#define WEBSITE "publicdesignworkshop.net"
String repository = "/foraging/core/php/";
uint32_t ip;

// Create sensor pin(s), value variable(s), sensor ID
int sensorPin = A0;
int sensorValue = 0;
String sensorID = "BS001";  //bend sensor 001

// END THINGS THAT NEED CUSTOMIZATION////////////////////////////////////////////////


Adafruit_CC3000_Client client;

void setup(void)
{
  //establish serial port
  Serial.begin(115200);

  //check to see whether the cc3000 is initialized
  Serial.print(F("Initializing..."));
  if(!cc3000.begin()) {
    Serial.println(F("failed. Check your wiring?"));
    return;
  }

  Serial.print(F("OK.\r\nConnecting to network..."));
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("connected!"));

  Serial.print(F("Requesting address from DHCP server..."));
  for(t=millis(); !cc3000.checkDHCP() && ((millis() - t) < dhcpTimeout); delay(500)) {
    Serial.println("....waiting");
  }
  if(cc3000.checkDHCP()) {
    Serial.println(F("OK"));
  } 
  else {
    Serial.println(F("failed"));
    return;
  }
  
  ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    else{
      Serial.println(F("resolved!"));
    }
    delay(500);
  }
}

void loop(void)
{
  //Begin the loop by reading the sensorPin
  sensorValue = analogRead(sensorPin);

  // Transform to String
  String bend = String(sensorValue);
  Serial.println("Sensor Reading: "+bend);

  //Open Socket
  Serial.println("...Connecting to server");
  t = millis();
  do {
    //client = cc3000.connectTCP(ip, port);
    client = cc3000.connectTCP(ip, 80);
  } 
  while((!client.connected()) &&
    ((millis() - t) < connectTimeout));
  // Send request
  if (client.connected()) {
    Serial.println("Connected"); 
    String request = "GET "+ repository + "createBend.php?sid=" + sensorID + "&value=" + bend + " HTTP/1.1\r\nHost: " + WEBSITE + "\r\n\r\n";
    Serial.print("...Sending request:");
    Serial.println(request);
    send_request(request);
  } 
  else {
    Serial.println(F("Connection failed"));    
    return;
  }
  Serial.println("...Reading response");
  show_response();

  Serial.println(F("Cleaning up..."));
  Serial.println(F("...closing socket"));
  client.close();
  
  
  //wait some amount of time before sending temperature/humidity to the PHP service.
  //TODO: SLEEP/WAKE 
  delay(60000);

}
/*******************************************************************************
 * send_request
 ********************************************************************************/
bool send_request (String request) {
  // Transform to char
  char requestBuf[request.length()+1];
  request.toCharArray(requestBuf,request.length()); 
  // Send request
  if (client.connected()) {
    client.fastrprintln(requestBuf);
  } 
  else {
    Serial.println(F("Connection failed"));    
    return false;
  }
  return true;
  free(requestBuf);
}
/*******************************************************************************
 * show_response
 ********************************************************************************/
void show_response() {
  Serial.println(F("--------------------------------"));
  while (client.available()) {
    // Read answer and print to serial debug
    char c = client.read();
    Serial.print(c);
  }
}
/*******************************************************************************
 * floatToString()
 ********************************************************************************/
// Float to String conversion
String floatToString(float number) {
  //  dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
  dtostrf(number,5,2,buffer);
  return String(buffer);

}
/*******************************************************************************
 * timedRead()
 ********************************************************************************/
// comment from Adafruit's GeoLocation sketch:
// Read from client stream with a 5 second timeout.  Although an
// essentially identical method already exists in the Stream() class,
// it's declared private there...so this is a local copy.
char timedRead(void) {
  unsigned long start = millis();
  while((!client.available()) && ((millis() - start) < responseTimeout));
  return client.read();  // -1 on timeout
}
