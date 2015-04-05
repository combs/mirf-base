<?php
chdir(dirname(__FILE__));
date_default_timezone_set('America/New_York');
ini_set('default_socket_timeout', 15);

$from=$argv[1];
$base="BASES";
$dir=dirname(__FILE__);
if (strpos($argv[0],"commands")) {
	$base_dir=preg_replace("/[^\/]*commands\/.*/","",dirname($argv[0]));
} else {
	$base_dir=dirname($argv[0]) . "/../";
	if (realpath($base_dir)=="") {
		$base_dir=".";
	}
	
}

$binaries_dir=$base_dir . "binaries/" . $FROM . "/";
$cache_dir="/tmp/cache-mirf-" . $FROM . "-files/";
$cache_file="/tmp/cache-mirf-" . $FROM;
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
		}
	
	} else {
		// echo "cache doesn't exist";
	}

}

?>