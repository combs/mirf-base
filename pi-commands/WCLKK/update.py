#!/usr/bin/python
from pyowm import OWM
from pprint import pprint
import os, sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__) ) )+ "/common")

from MirfBase import MirfBase

mirf = MirfBase()
mirf.try_cache(60*15)

degrees = str(chr(223))
apikey=None

with open(".api-key-openweathermap", "rb") as apifile:
    apikey = apifile.readline().strip()

owm = OWM(apikey)

wap = owm.weather_at_place('20017,US')
w = wap.get_weather()

pprint(w.get_temperature(unit='fahrenheit'))


outputConditions = "C"
outputNow = "N"
outputLater = "L"
output1 = "0"
output2 = "1"

status = w.get_status()
detailedStatus = w.get_detailed_status().capitalize()

outputConditions += detailedStatus
temp = w.get_temperature(unit='fahrenheit')
rain = w.get_rain()
snow = w.get_snow()
wind = w.get_wind()
min = temp['temp_min']
max = temp['temp_max']
now = temp['temp']

outputNow += str(int(now)) + degrees + " now."
outputLater += (str(int(min)) + "-" + str(int(max)) + degrees)

mirf.send_to_client(outputNow)
mirf.send_to_client(outputLater)
mirf.send_to_client(outputConditions)
