# AirThingy v2

AirThingy is a hardware sensor platform based on the Intel Edison hardware platform

To get started, assemble the parts and circuit outlined below, and run the included Python script.  That's it!

## Parts List:

- Intel Edison w/ Arduino Breakout
- SparkFun I2C Breakout Block for Edison
- Sharp GP2Y1010AU0F Dust Sensor (tested with the breakout board from WaveShare)
- MICS-6814 multi-channel Gas Sensor
- TH02 Temperature and Humidity Sensor
- A bunch of wires

## Setup for Edison

0. Update your Edison firmware! See the installer for the Edison software package for details.

1. Install the Sparkfun I2C block onto the Arduino board, giving access to a second I2C bus.

2. Connect the ILED pin of the Dust Sensor to Pin 8 on the Arduino

3. Connect the AO Pin of the Dust sensor to Analog Pin A0 on the Arduino

4. Connect the TH02 to the built-in I2C bus (pins SDA, SCL, 3.3V and GND)

5. Connect the MICS-6814 to the SparkFun I2C block

6. Copy the included Python script to your Edison

7. Run the script, enjoy!

# Notes

- This project uses UPM and MRAA to control GPIO.  MRAA does not normally let you access I2C Bus #1, exposed by the SparkFun I2C block.
We have provided a modified version that works here: http://github.com/scintilla-aircheck/mraa

- Until the drivers we produced or adapted for some of these devices are pulled into upstream UPM, you can find our version of UPM here: http://github.com/scintilla-aircheck/upm

- TH02 does NOT play nice with others.  Specifically, it will produce incorrect (-50C) readings if it isn't the only device on the I2C bus!
  This is why the extra bus is required

- If you are using a I2C LCD (such as the one included with the Grove kit), or other components at 5V, you'll want a TCA9545A I2C bus Mux.

