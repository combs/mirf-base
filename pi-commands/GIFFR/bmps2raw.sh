#!/bin/bash

COMMAND='avconv'
type avconv >/dev/null 2>/dev/null || type ffmpeg >/dev/null 2>/dev/null  && COMMAND='ffmpeg' || { echo >&2 "Requires ffmpeg or avconv but neither is installed.  Aborting."; exit 1; }
COMMAND='ffmpeg'
cd `dirname $1`
for arg in $1*.bmp

do echo -n "";

outfile="$2`basename $arg|sed s:\....$:.raw:`"

$COMMAND -y -loglevel quiet -vcodec bmp -i $arg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 $outfile  2>/dev/null >/dev/null && rm $arg

done
