<?php
date_default_timezone_set('America/New_York');
ini_set('default_socket_timeout', 15);


$base="BASES";
$dir=dirname(realpath($_SERVER["PHP_SELF"]));

if (strpos($argv[0],"commands")) {
	$base_dir=preg_replace("/[^\/]*commands\/.*/","",dirname($argv[0]));
} else {
	$base_dir=dirname($argv[0]) . "/../";
	if (realpath($base_dir)=="") {
		$base_dir=$dir;
	}
	
}

if ($argv[1] && "$argv[1]" != "") {
	$from=$argv[1];
} else {
	$from=basename($dir);
}


$binaries_dir=$base_dir . "binaries/" . $from . "/";
$cache_dir="/tmp/cache-mirf-" . $from . "-files/";
$cache_file="/tmp/cache-mirf-" . $from;
$errors=0;

array_shift($argv);
array_shift($argv);

$details=$argv;

// Aliases 

$FROM=&$from;
$BASEDIR=&$base_dir;
$DIR=&$dir;
$DETAILS=&$details;
$BASE=&$base;


// Utility functions

function try_cache($the_file, $max_age) {		
	if (file_exists($the_file)) {
		if (time() - filemtime($the_file) < $max_age) {
			$lines=file($the_file);
			foreach ($lines as $line_num => $line) {
				echo $line;
				usleep(50000);
			}
			exit;			
		}
		else {
			// echo "cache too old";
			unlink($the_file);
		}
	} else {
		// echo "cache doesn't exist";
	}
}

function cached_echo($the_string) {
	global $cache_file;
	file_put_contents($cache_file, $the_string, FILE_APPEND | LOCK_EX);
	echo ($the_string);
}

function send_to_client($the_string) {
	global $FROM, $BASE;
	cached_echo($FROM . $BASE . $the_string . "\n");
	usleep(50000); // breather for base station.
}


function map($value, $fromLow, $fromHigh, $toLow, $toHigh) {
    $fromRange = $fromHigh - $fromLow;
    $toRange = $toHigh - $toLow;
    $scaleFactor = $toRange / $fromRange;

    // Re-zero the value within the from range
    $tmpValue = $value - $fromLow;
    // Rescale the value to the to range
    $tmpValue *= $scaleFactor;
    // Re-zero back to the to range
    return $tmpValue + $toLow;
}

function encodeColorChannel($value) {
	$value=$value >> 2;
	$value+=32;
	return chr($value);
}

function encodeColor($red,$green,$blue) {
	$text="";
	$text .= encodeColorChannel($red * 0.8);
	$text .= encodeColorChannel($green * 1.0);
	if ($blue > 6) {
		$blue = $blue * 0.6;
	} 	
	$text .= encodeColorChannel($blue);	
	return $text;
}
	

// Change path and leave

chdir($dir);

?>