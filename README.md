# AirThingy

AirThingy is a hardware sensor platform based on Arduino and the ESP8266 Wifi platform.

It uses an Atmel-based Arduino for interfacing with gas sensors and dust sensors, and the ESP8266's fantastic network capabilities to send it off to the cloud.
The two share data over the I2C bus.

To get started, flash the SensorThingy sketch onto an Arduino Uno or compatible, and the
AirThingy sketch onto an ESP8266 or compatible.

## Parts List:

- Arduino Uno R2 or R3
- ESP8266 board (tested with SparkFun Thing Dev)
- Sharp GP2Y1010AU0F Dust Sensor (tested with the breakout board from WaveShare)
- MQ-135 Gas Sensor (tested with generic breakout board -- Olimex or equivalent)
- Temperature and Humidity Sensor
- A bunch of wires
- An external 5V PSU (One of those snap-on breadboard PSUs will do)

NOTE: you absolutely NEED a real external 5V PSU, because the MQ-135 will blow up everything with its 200mA power draw!!

## Setup

1. Connect the VIN/GND of the Arduino and ESP8266 to 5V/GND on your PSU.  Similarly, connect VCC and GND on the Dust Sensor, Humidity Sensor, and MQ-135 to 5V and GND on the PSU.

2. Connect the ILED pin of the Dust Sensor to Pin 7 on the Arduino

3. Connect the AO Pin of the Dust sensor to Analog Pin A0 on the Arduino

4. Connect the AO Pin of the MQ-135 ssensor to Analog Pin A1 on the Arduino

5. Connect the AO pin on the Humidity Sensor to Analog Pin A2 on the Arduino

6. Connect pins A4 and A5 (I2C SDA and SCLK) to pins 2 and 14 on the ESP8266 (these are really I2C, if you read deep in the docs)

7. Edit the AirThingy.ino sketch with your Wifi SSID and Password.  Also edit SensorThingy.ino with your latitude/longitude

8. You need to calibrate MQ-135 for best results.  See https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library for details

9. You'll need a stack of Arduino libraries to build the sketches.  library-snapshot.tgz has them, but if you want to fetch them yourself, you need:
	- The ESP8266 core package (https://github.com/esp8266/Arduino)
	- ArduinoJson
	- Time
	- MQ135 (https://hackaday.io/project/3475-sniffing-trinket/log/12363-mq135-arduino-library)

That's it! Boot it, and watch as live data is sent to the cloud!


