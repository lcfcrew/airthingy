#!/usr/bin/python
"""

AirThingy -- Log and process air quality data
 Uses the Intel Edison/Galileo platform and UPM
 By edg and the LCF Crew
 5/8/16


Setup:
 1. Update your firmware!
 Get a shell on the Edison, and run 'configure_edison --upgrade'
 Next, you should probably run `opkg update` and `opkg upgrade`
 NOTE: WARNING: FIXME: As of 5/10/16, the MICS1814 driver does not
 exist in UPM! It is stuck in a pull request on GitHub! For now, I have 
included a copy of the driver in
 this repo.  Drop the "mics6814" folder in a upm source tree, and build 
it first.
 Hopefully this won't be needed soon
 2. Connect the follwoing components to your Edison (most likely using a 
Grove Base shield)
    - MICS-6814 (aka. Grove Multichannel Gas Sensor) via I2C
    - TH02 (aka. Grove Temp/humidity sensor Accurate&Mini)
    - Sharp GPYx Dust sensor
      This one isn't super Grove-friendly, try using the headers (A0, 
3.3v, D7) directly
    - (optional) I2C LCD
       NOTE: WARNING: WTF: This needs 5V, and everything else needs
             3.3v on their i2c bus! On Grove, this means you need to
             splice out the power leads on your I2C connector and
             route them to 5V/GND on the main headers!
"""
### CONFIGURATION ###

I2C_BUS = 1 # Edison uses Bus #6, change for other platforms
TH02_I2C_BUS = 6
POLL_RATE = 15
# I2C Mux channels for the devices
LCD_I2C_CHANNEL = 0
MICS6814_I2C_CHANNEL = 1
TH02_I2C_CHANNEL = 2
DUST_APIN = 0
DUST_DPIN = 8
DUST_AREF = 5000

from pyupm_tca9545a import TCA9545A as I2CMux
from pyupm_i2clcd import Jhd1313m1 as I2CLCD
from pyupm_mics6814 import MICS6814 as GasSensor
from pyupm_th02 import TH02 as THSensor
from pyupm_sharpdust import SharpDust as DustSensor
from time import sleep
import aqi
import threading

def printTop(lcd,str):
    mux.selectBus(LCD_I2C_CHANNEL)
    lcd.setCursor(0,0)
    str += " " * (16 - len(str))
    lcd.write(str)

def printBottom(lcd,str):
    mux.selectBus(LCD_I2C_CHANNEL)
    lcd.setCursor(1,0)
    str += " " * (16 - len(str))
    lcd.write(str)

def printTemp(temp):
    printBottom(lcd,"TEMP: %02fC" % temp)
    print "Temperature:", temp, "C"
def printHumidity(humidity):
    printBottom(lcd,"HUM: %02f%%" % humidity)
    print "Humidity:", humidity, "%%"
def printCO(co):
    printBottom(lcd,"CO: %02fPPM" % co)
    print "CO:", co, "ppm"
def printNH3(nh3):
    printBottom(lcd,"NH3: %02fPPM" % nh3)
    print "NH3:", nh3, "ppm"
def printNO2(no2):
    printBottom(lcd,"NO2: %02fPPM" % no2)
    print "NO2:", no2, "ppm"
def printDust(dust_density):
    printBottom(lcd,"DUST: %02fug/m^3" % dust_density)
    print "Dust:", dust_density, "ug/m^3"
def printAQI(score):
    print "AQI:", score
    printBottom(lcd,"AQI: %d" % score)

def calculate_aqi(co,no2,dust_density):
    factors = []
    if co > 0:
        factors.append((aqi.POLLUTANT_CO_8H,co))
    if no2 > 0:
        factors.append((aqi.POLLUTANT_NO2_1H,no2))
    if dust_density > 0 and dust_density < 400:
        factors.append((aqi.POLLUTANT_PM25,dust_density))
    try:
        return aqi.to_aqi(factors)
    except:
        return -99


def do_dust():
    global dust_density
    while True:
        if dust:
            dust_density = dust.value(DUST_AREF)


def loop():

    dthread = threading.Thread()
    dthread.run(do_dust)
    while True:
        try:
            temp = -99.0
            humidity = -99.0
            dust_density = -99.0
            co = -99.0
            nh3 = -99.0
            no2 = -99.0
            # Measure the TH02
            if th:
                #mux.selectBus(TH02_I2C_CHANNEL)
                temp = th.getTemperature()
                humidity = th.getHumidity()
                if humidity < 0:
                    humidity = "ERR"
            if gas:
                mux.selectBus(MICS6814_I2C_CHANNEL)
                co = gas.measure_CO()
                if co < 0:
                    co = "ERR"
                nh3 = gas.measure_NH3()
                if nh3 < 0:
                    nh3 = "ERR"
                no2 = gas.measure_NO2()
                if no2 < 0:
                    no2 = "ERR"
            
            wait = POLL_RATE / 6
            printTemp(temp)
            sleep(wait)
            printHumidity(humidity)
            sleep(wait)
            printCO(co)
            sleep(wait)
            printNH3(nh3)
            sleep(wait)
            printNO2(no2)
            sleep(wait)
            printDust(dust_density)
            sleep(wait)
            printAQI(calculate_aqi(co,no2,dust_density))
        except:
            raise
            sleep(POLL_RATE)

def lcd_init():
    global lcd
    # bring up the LCD
    mux.selectBus(LCD_I2C_CHANNEL)
    lcd = I2CLCD(I2C_BUS)

def gas_init():
    global gas
    # Bring up the MICS-6814
    print "Starting MICS6814"
    printBottom(lcd,"MICS6814 INIT")
    mux.selectBus(MICS6814_I2C_CHANNEL)
    gas = GasSensor(I2C_BUS)
    sleep(1)
    print "Heating up MICS6814"
    mux.selectBus(MICS6814_I2C_CHANNEL)
    gas.powerOn()
    printBottom(lcd,"MICS6814 PREHEAT")
    sleep(6)

def th02_init():
    global th
    # Bring up the TH02
    print "Starting TH02"
    printBottom(lcd,"TH02 INIT")
    #mux.selectBus(TH02_I2C_CHANNEL)
    th = THSensor(TH02_I2C_BUS)


def dust_init():
    global dust
    printBottom(lcd,"DUST INIT")
    dust = DustSensor(DUST_APIN,DUST_DPIN)
    sleep(1)
if __name__ == '__main__':
    global mux
    print "AirThingy Starting up....."
    # Start the mux
    mux = I2CMux(I2C_BUS)
    lcd_init()
    printTop(lcd,"  AIRTHINGY V2  ")
    printBottom(lcd,"      BOOT      ")
    th02_init()
    dust_init()
    gas_init()
    sleep(1)

    loop()
