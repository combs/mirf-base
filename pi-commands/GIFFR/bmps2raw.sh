#!/bin/bash
cd `dirname $0`
for arg in $1*.bmp
do ./png2fb.py $arg `echo $arg|sed s:\....$:.raw:`
done
