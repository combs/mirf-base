#!/usr/bin/env python
# encoding: utf-8

from __future__ import print_function

import sys, os, inspect, time

base="BASES"

class MirfBase(object):

    def __init__(self):
        self.base="BASES"
        frm = inspect.stack()[1]
        mod = inspect.getmodule(frm[0])
        self.whatsapp="test"
        callerModule = os.path.realpath(mod.__file__)
        self.fro = os.path.basename(os.path.dirname(callerModule))
        sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
        argv = sys.argv
        if (len(argv) > 1):
            self.fro = argv[1]
            argv.pop(0)
            argv.pop(0)
        self.details = []
        if (len(argv) > 0):
            self.details = argv

    def get_cache_dir(self):
        return "/tmp/cache-mirf-" + self.fro + "-files/"


    def get_cache_file(self):
        return "/tmp/cache-mirf-" + self.fro

    def try_cache(self, freshness):
        try:
            mtime = os.path.getmtime(self.get_cache_file())
        except OSError:
            return

        if (mtime > time.time() - freshness):
            with open(self.get_cache_file(), "rb") as cacheFile:
                print(cacheFile.read())
            sys.exit(0)

        os.unlink(self.get_cache_file())

    def cached_write(self, text):
        with open(self.get_cache_file(), "a+") as cacheFile:
            cacheFile.write(text)
            print(text, end="")

    def send_to_client(self, text):
        self.cached_write(self.fro + self.base + text + "\n")
        time.sleep(0.0001)

    def translate( self, value, leftMin, leftMax, rightMin, rightMax):
        # Figure out how 'wide' each range is
        leftSpan = leftMax - leftMin
        rightSpan = rightMax - rightMin
        if (value < leftMin):
            value = leftMin
        if (value > leftMax):
            value = leftMax
        # Convert the left range into a 0-1 range (float)
        valueScaled = float(value - leftMin) / float(leftSpan)
        # Convert the 0-1 range into a value in the right range.
        return rightMin + (valueScaled * rightSpan)





if __name__ == '__main__':
    mirf = MirfBase()
    mirf.try_cache(5)




# $dir=dirname(realpath($_SERVER["PHP_SELF"]));
#
# if (strpos($argv[0],"commands")) {
# 	$base_dir=preg_replace("/[^\/]*commands\/.*/","",dirname($argv[0]));
# } else {
# 	$base_dir=dirname($argv[0]) . "/../";
# 	if (realpath($base_dir)=="") {
# 		$base_dir=$dir;
# 	}
#
# }
#
# if (isset($argv[1]) && "$argv[1]" != "") {
# 	$from=$argv[1];
# } else {
# 	$from=basename($dir);
# }
#
#
# $binaries_dir=$base_dir . "binaries/" . $from . "/";
# $cache_dir="/tmp/cache-mirf-" . $from . "-files/";
# $cache_file="/tmp/cache-mirf-" . $from;
# $errors=0;
#
# array_shift($argv);
# array_shift($argv);
#
# $details=$argv;
#
# // Aliases
#
# $FROM=&$from;
# $BASEDIR=&$base_dir;
# $DIR=&$dir;
# $DETAILS=&$details;
# $BASE=&$base;
#
#
# // Utility functions
#
# function try_cache($the_file, $max_age) {
# 	if (file_exists($the_file)) {
# 		if (time() - filemtime($the_file) < $max_age) {
# 			$lines=file($the_file);
# 			foreach ($lines as $line_num => $line) {
# 				echo $line;
# 				usleep(50000);
# 			}
# 			exit;
# 		}
# 		else {
# 			// echo "cache too old";
# 			unlink($the_file);
# 		}
# 	} else {
# 		// echo "cache doesn't exist";
# 	}
# }
#
# function cached_echo($the_string) {
# 	global $cache_file;
# 	file_put_contents($cache_file, $the_string, FILE_APPEND | LOCK_EX);
# 	echo ($the_string);
# }
#
# function send_to_client($the_string) {
# 	global $FROM, $BASE;
# 	cached_echo($FROM . $BASE . $the_string . "\n");
# 	usleep(50000); // breather for base station.
# }
#
#
# function map($value, $fromLow, $fromHigh, $toLow, $toHigh) {
#     $fromRange = $fromHigh - $fromLow;
#     $toRange = $toHigh - $toLow;
#     $scaleFactor = $toRange / $fromRange;
#
#     // Re-zero the value within the from range
#     $tmpValue = $value - $fromLow;
#     // Rescale the value to the to range
#     $tmpValue *= $scaleFactor;
#     // Re-zero back to the to range
#     return $tmpValue + $toLow;
# }
#
# function encodeColorChannel($value) {
# 	$value=$value >> 2;
# 	$value+=32;
# 	return chr($value);
# }
#
# function encodeColor($red,$green,$blue) {
# 	$text="";
# 	$text .= encodeColorChannel($red );
# 	$text .= encodeColorChannel($green );
# 	$text .= encodeColorChannel($blue);
# 	return $text;
# }
#
#
# // Change path and leave
#
# chdir($dir);
#
# ?>
