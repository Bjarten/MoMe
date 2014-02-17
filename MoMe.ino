
 /***************************MoMe*********************************
 Opretta av: Bjarte Mehus Sunde 17.02.2014
 Kommentar: Dette er koden til MoMe, det magiske armbåndet. 
 *****************************************************************/

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
//#include "DHT.h"             
#include<stdlib.h>


// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   1      
#define ADAFRUIT_CC3000_VBAT  9
#define ADAFRUIT_CC3000_CS    12

// Soil sensor pins                    //   
const uint8_t dataPin  =  6;           // 
const uint8_t clockPin =  10;          //
                                       // Kode frå eit eksempel, kan fjernes seinare
// Soil sensor variables               // 
float t;                               //
float h;                               //
float dewpoint;                        //



// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed. Klokkehastigheten på Flora er 4 Mhz
                                
// WLAN parameters
#define WLAN_SSID       "AndroidAP"    // WiFi-hotspot fra mobiltelefon
#define WLAN_PASS       "ekgf4435"     // Koden til hotspot

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Carriots parameters
#define WEBSITE  "api.carriots.com"
#define API_KEY "3c549d4731c06cd837736ecad683ad5db2ca7e87e8726663a8914352b6943f3b"
#define DEVICE  "MoMe@Bjarte"

uint32_t ip;

void setup(void)
{
  // Initialize
  Serial.begin(9600);
  
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
 
}

void loop(void)
{
  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }  
 
  // Get the website IP & print it
  ip = 0;
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }
  cc3000.printIPdotsRev(ip);
  
// Convert Floats to Strings.
  char TempString[32];  //  A temporary character array to hold data.
  // dtostrf( [Float variable] , [Minimum SizeBeforePoint] , [sizeAfterPoint] , [WhereToStoreIt] )
  dtostrf(t,2,1,TempString);
  String temperature =  String(TempString);
  dtostrf(h,2,2,TempString);
  String humidity =  String(TempString);

  // Prepare JSON for Xively & get length
  int length = 0;

  String data = "{\"protocol\":\"v2\",\"device\":\""+String(DEVICE)+"\",\"at\":\"now\",\"data\":{\"Temperature\":"+String(temperature)+",\"Humidity\":"+String(humidity)+"}}";
  
  length = data.length();
  Serial.print("Data length");
  Serial.println(length);
  Serial.println();
  
// Print request for debug purposes
//  Serial.println("POST /streams HTTP/1.1");
//  Serial.println("Host: api.carriots.com");
//  Serial.println("Accept: application/json");
//  Serial.println("User-Agent: Arduino-Carriots");
//  Serial.println("Content-Type: application/json");
//  Serial.println("carriots.apikey: " + String(API_KEY));
//  Serial.println("Content-Length: " + String(length));
//  Serial.print("Connection: close");
//  Serial.println();
//  Serial.println(data);
//  


// Send request
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.println(F("Connected!"));
    client.println(F("POST /streams HTTP/1.1"));
    client.println(F("Host: api.carriots.com"));
    client.println(F("Accept: application/json"));
    client.println(F("User-Agent: Arduino-Carriots"));
    client.println(F("Content-Type: application/json"));
    client.print(F("carriots.apikey: "));
    client.println(String(API_KEY));
    client.print(F("Content-Length: "));
    client.println(String(length));
    client.println(F("Connection: close"));
    client.println();
    client.println(data);
  } 
  else {
    Serial.println(F("Connection failed"));    
    return;
  }
  
  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  client.close();
  
  Serial.println(F("-------------------------------------"));
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
  
  // Wait 10 seconds until next update
  //  delay(10000);
   
} // loop

