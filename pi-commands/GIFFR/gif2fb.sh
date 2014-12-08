#!/bin/bash
avconv -y -loglevel quiet -vcodec gif -i $1 -vf scale=320:240 -vcodec rawvideo -f rawvideo -pix_fmt rgb565 `echo $1|sed s:\....$:\%02d.raw:`
