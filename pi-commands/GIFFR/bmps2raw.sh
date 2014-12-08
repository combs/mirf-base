#!/bin/bash
COMMAND='avconv'
type avconv >/dev/null 2>&1 || type ffmpeg >2/dev/null 2>&1 && COMMAND='ffmpeg' || { echo >&2 "Requires ffmpeg or avconv but neither is installed.  Aborting."; exit 1; }
cd `dirname $0`
for arg in $1*.bmp
do $COMMAND -y -loglevel quiet -stats off -vcodec bmp -i $arg -vcodec rawvideo -f rawvideo -pix_fmt rgb565 `echo $arg|sed s:\....$:.raw:` 2>/dev/null && rm $arg
done
