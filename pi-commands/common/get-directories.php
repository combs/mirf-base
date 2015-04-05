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
}


// Change path and leave

chdir(__DIR__);

?>