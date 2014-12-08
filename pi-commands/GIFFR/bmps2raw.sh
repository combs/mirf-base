#!/bin/bash
cd `dirname $0`
for arg in $1*.bmp
do avconv -y -loglevel quiet -vcodec bmp -i $arg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 `echo $arg|sed s:\....$:.raw:`
done
