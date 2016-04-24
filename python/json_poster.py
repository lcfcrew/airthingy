from time import sleep
import serial
import sys
import json
from requests import post
import datetime


the_url = "http://airchecknasa.com/api/v1/data_points/"

def _do_json_post(url,data):
        resp = post(the_url,json=data)
        print "Code: ", resp.status_code

ser = serial.Serial(sys.argv[1], 9600) # Establish the connection on a specific port
counter = 32 # Below 32 everything in ASCII is gibberish
while True:
     indata =  ser.readline() # Read the newest output from the Arduino
     data = json.loads(indata)
     data['time'] = str(datetime.datetime.now())
     print data
     _do_json_post(the_url,data)
     sleep(.1) # Delay for one tenth of a second


