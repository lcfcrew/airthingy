/*
 * SnesorThingy -- Sensors/i2c streaming platoform
 * 
 * By edg and the LCF Crew
 * 
 * This sketch implements a sensor platform, that streams various sensor data
 * through I2C to a network device, which can send it to the cloud
 * 
 * This is the "slave" i2c device.  For the master, see the "AirThingy"
 * in this repo.
 */

// CHANGE ME: Pin definitions for sensors!
#define SENSOR1_NAME "dust"
#define SENSOR1_PIN 10

#define SENSOR2_NAME = "air_quality"
#define SENSOR2_PIN 11
// TODO: Add more?

// The i2c address of this board
#define MY_I2C_ADDR 69

// Geo info
double lat = 34.4140067; // Seclab
double lon = -119.8427564;

#include <ArduinoJson.h> // Oh yeah, we're doing that.  Get ready...
#include <Wire.h> // Man, I feel really pro using I2C for a project now...

void setup() {
  Serial.begin(9600);
  Serial.println("### SensorThingy v0 BOOT:");
  Wire.begin(MY_I2C_ADDR);
  Serial.println("I2C Address: " + String(MY_I2C_ADDR));
  Wire.onRequest(requestEvent); // register event

  Serial.println("Waiting for Master...");
}

void loop() {
  // Unused
}


// Callback for I2C -- Reads sensors and returns a JSON
void requestEvent() {
  char toSend[200];
  Serial.println("Got request from Master");
  memset(toSend,0,sizeof(toSend));
  StaticJsonBuffer<200> jsonBuffer; // Woah, these dudes do malloc-less json?? Hax
  JsonObject& data = jsonBuffer.createObject();
  data["lat"] = lat;
  data["lon"] = lon;
  data["dust"] = 0.420; // TODO: FAKE
  data["air_quality"] = 0.6969; // TODO: FAKE
  data.prettyPrintTo(toSend, sizeof(toSend));
  Wire.write(toSend,sizeof(toSend));
}
