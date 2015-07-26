#!/bin/bash

ping -q -c 1 comcast.com >/dev/null 2>/dev/null
if [ $? -gt 0 ] 
then

curl 'http://dc.chriscombs.net:10080/color.php?speed=1&mode=p&color0=%23500a00&color1=%23000000' >/dev/null 2>/dev/null
touch /tmp/cache-mirf-internet-health

elif [ -e /tmp/cache-mirf-internet-health ]
then
curl 'http://dc.chriscombs.net:10080/color.php?speed=1&mode=f&color0=%230e4d00&color1=%23000000' >/dev/null 2>/dev/null
rm /tmp/cache-mirf-internet-health
fi


