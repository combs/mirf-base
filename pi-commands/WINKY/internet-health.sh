#!/bin/bash
cd `dirname $0`
. ../common/get-directories

ping -q -c 1 comcast.com >/dev/null 2>/dev/null
if [ $? -gt 0 ] 
then

echo 'WINKYBASESp4"    ' > $BASEDIR/out/internet-health
touch /tmp/cache-mirf-internet-health

elif [ -e /tmp/cache-mirf-internet-health ]
then
echo 'WINKYBASESf#3    ' > $BASEDIR/out/internet-health

rm /tmp/cache-mirf-internet-health
fi


