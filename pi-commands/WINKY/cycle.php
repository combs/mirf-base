<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";

$stringOutput="C";
$stringOutput.=$details[0];
while ("$details[1]" != "") {
	$stringOutput.=encodeColor($details[1],$details[2],$details[3]);
	
	array_shift($details);
	array_shift($details);
	array_shift($details);
}
 

send_to_client($stringOutput);

?>