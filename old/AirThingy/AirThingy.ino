/*
 * AirThingy -- Receive Sensor Data over I2C, send it out via a POST request
 * 
 * by edg and the LCF Crew
 * 
 * This is the I2C Master, which should run on an ESP8266.
 * For the slave with the sensors on it, go see SensorThingy
 */

#include <ESP8266WiFi.h> // Oh yeah... wifi ftw
#include <WiFiClientSecure.h> // Secure? I don't think so, but OK
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h> // for POSTing
#include <WiFiUdp.h> // for NTP
#include <Wire.h> //This sketch has a very dominant personality
#include <ArduinoJson.h> //Why am I doing JSON over I2C? I dunno really...

const char* ssid = "SSID";
const char* password = "fuckyou123";

// The master polling interval
const int interval = 6000;

// Send the data here
#define API_ENDPOINT "http://airchecknasa.com/api/v1/data_points/"

// TODO: This
// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "CF 05 98 89 CA FF 8E D8 5E 5C E0 C2 E4 F7 E6 C3 C7 50 DD 5C";

// Stuff for NTP
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Local port to use
unsigned int localPort = 2390;      // local port to listen for UDP packets


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

// Helper to fetch the current NTP time in seconds
int getNTPTime() {
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {}
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println("[NTP] Current time is: " + String(epoch));
  }
}


// Do a POST with some JSON
void doPost(String data) {

  Serial.print("requesting URL: ");
  Serial.println(API_ENDPOINT);
  HTTPClient http;
  http.begin(API_ENDPOINT);
  http.setUserAgent("AirThingy/v0");
  http.addHeader("Content-Type","application/json",false);
  int code = http.POST(data);
  String resp = http.getString();
  Serial.println(String(String(code) + ": " + resp));
  
}


int done = 0;
int len = 0;
char msgbuf[200];

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  
  Serial.println("Got i2c Event");
  if (done == 1) {
    done = 0;
    len = 0;
  }
  while (Wire.available()) { // loop through all 
    char c = Wire.read(); // receive byte as a character
    msgbuf[len] = c;         // print the character
    if (c == '\0') {
      done = 1;
      Serial.println("Done");
      //Serial.print(msgbuf);
    }
    len++;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("### AirThingy v0 BOOT:");
  Serial.println("[wifi] Connecting to Wifi: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("[wifi] ");
  Serial.println("[wifi] WiFi connected");
  Serial.println("[wifi] IP address: " + WiFi.localIP());
  Serial.println("[ntp] Starting NTP");
  udp.begin(localPort);
  Serial.print("[ntp] Local port: ");
  Serial.println(udp.localPort());
  getNTPTime();
  Serial.println("[i2c] Bringing up I2C Master...");
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Every n seconds, poll the slave for sensor data
  //char data[200];
  delay(100);
  //String fake1 = " {\"type\": \"dust\", \"value\": \"5.4535\", \"date\": \"2016-04-20T04:05:59\", \"latitude\": \"11.2342342\", \"longitude\": \"21.534534\"}";
  //doPost(fake1);
  //delay(interval);
}


