<?php
chdir(dirname(__FILE__));
date_default_timezone_set('America/New_York');
ini_set('default_socket_timeout', 15);

$cache_file="/tmp/cache-mirf-" . $FROM;
$cache_dir="/tmp/cache-mirf-" . $FROM . "-files/";

$from=$argv[1];


$base_dir=preg_replace("/[^\/]*commands\/.*/","",dirname($argv[0]));

$dir=dirname(__FILE__);

$binaries_dir=$base_dir . "binaries/" . $FROM . "/";

array_shift($argv);
array_shift($argv);

$details=$argv;


$FROM=&$from;
$BASE=&$base_dir;
$DIR=&$dir;
$DETAILS=&$details;


?>