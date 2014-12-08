#!/bin/bash
avconv -y -loglevel quiet -vcodec bmp -i $1 -vcodec rawvideo -f rawvideo -pix_fmt rgb565 `echo $1|sed s:\....$:.raw:`
