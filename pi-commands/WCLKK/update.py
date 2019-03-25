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
tempnow = temp['temp']
tempmin = temp['temp_min']
tempmax = temp['temp_max']

if fc:
    w = fc.get_forecast().get_weathers()[0]
    temp = w.get_temperature(unit='fahrenheit')
    # print(temp)
    tempmin = temp['min']
    tempmax = temp['max']


rain = w.get_rain()
snow = w.get_snow()
wind = w.get_wind()

conditions = []

bestCondition = detailedStatus

if len(snow) > 0:
    # print(snow)
    conditions.append(str(max(snow.values())) + '" snow')
if len(rain) > 0:
    # print(rain)
    conditions.append(str(max(rain.values())) + '" rain')

conditions.append(detailedStatus)
conditions.append(status)

length = 0
acceptedConditions = []
for condition in conditions:
    if len(condition) + length + (len(acceptedConditions) * 2) < 16:
        length += len(condition)
        acceptedConditions.append(condition)

outputConditions = "C" + ", ".join(acceptedConditions)

outputNow += str(int(tempnow)) + degrees + " now."
outputLater += (str(int(tempmin)) + "-" + str(int(tempmax)) + degrees)

# mirf.send_to_client(output1)
# mirf.send_to_client(output2)
mirf.send_to_client(outputNow)
mirf.send_to_client(outputLater)
mirf.send_to_client(outputConditions)
