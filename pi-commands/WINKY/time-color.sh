#!/bin/bash
cd `dirname $0`
. ../common/get-directories

php time-color.php > /tmp/mirf-time-color && mv /tmp/mirf-time-color /var/local/nrf24/out/time-color
