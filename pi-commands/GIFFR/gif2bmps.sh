#!/bin/bash

convert "$1" -coalesce -resize 320x240 -background black -gravity center -extent 320x240 -depth 24 -alpha off -rotate 90 "$2%02d.bmp"


