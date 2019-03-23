#!/usr/bin/python
from pyowm import OWM
from pprint import pprint
import os, sys
mydir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.dirname(mydir))
sys.path.append(os.path.dirname(mydir)+ "/common")

from MirfBase import MirfBase

mirf = MirfBase()
mirf.try_cache(60*15)

degrees = str(chr(223))
apikey=None

with open(mydir + "/.api-key-openweathermap", "rb") as apifile:
    apikey = apifile.readline().strip()

owm = OWM(apikey)

placename='20017,US'

wap = owm.weather_at_place(placename)
w = wap.get_weather()
fc = owm.daily_forecast(placename, limit=1)


outputConditions = "C"
outputNow = "N"
outputLater = "L"
output1 = "0                "
output2 = "1                "

status = w.get_status()
detailedStatus = w.get_detailed_status().capitalize()

temp = w.get_temperature(unit='fahrenheit')
now = temp['temp']
min = temp['temp_min']
max = temp['temp_max']

if fc:
    w = fc.get_forecast().get_weathers()[0]
    temp = w.get_temperature(unit='fahrenheit')
    print(temp)
    min = temp['min']
    max = temp['max']


rain = w.get_rain()
snow = w.get_snow()
wind = w.get_wind()

conditions = []

bestCondition = detailedStatus

if len(snow) > 0:
    conditions.append(str(max(values(snow))) + '" snow')
if len(rain) > 0:
    conditions.append(str(max(values(rain))) + '" rain')

conditions.append(detailedStatus)
conditions.append(status)

length = 0
acceptedConditions = []
for condition in conditions:
    if len(condition) + length + (len(acceptedConditions) * 2) < 16:
        length += len(condition)
        acceptedConditions.append(condition)

outputConditions = "C" + ", ".join(acceptedConditions)

outputNow += str(int(now)) + degrees + " now."
outputLater += (str(int(min)) + "-" + str(int(max)) + degrees)

# mirf.send_to_client(output1)
# mirf.send_to_client(output2)
mirf.send_to_client(outputNow)
mirf.send_to_client(outputLater)
mirf.send_to_client(outputConditions)
