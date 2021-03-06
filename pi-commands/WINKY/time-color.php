<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

try_cache($cache_file,30);

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";


$times=array(0, 330, 420, 840, 1020, 1130, 1150, 1260, 1441);
$colors=array(  array(0,0,0),
			 	array(0,0,0),
			 	array(80,80,100),
			 	array(255,255,155),
			 	array(255,200,30),
			 	array(90,30,30),
			 	array(0,0,30),
			 	array(0,0,0),
			 	array(0,0,0)
			 	);

$count=count($times);

for($i=0; $i < ($count - 1); $i++) {
	if ( $currentMinute < $times[$i+1]   && 
		($currentMinute >= $times[$i] ) 
		)  {
		$desiredColor[0]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][0],$colors[$i+1][0]);
		$desiredColor[1]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][1],$colors[$i+1][1]);
		$desiredColor[2]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][2],$colors[$i+1][2]);
	}
	
	
}

$stringOutput="i1"; // cycle at slowest speed
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]);
$stringOutput.=encodeColor($desiredColor[0]*.98,$desiredColor[1]*.95,$desiredColor[2]*.95);
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]*.92);
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]*.92);
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]);
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]);
$stringOutput.=encodeColor($desiredColor[0]*.94,$desiredColor[1]*.92,$desiredColor[2]);
$stringOutput.=encodeColor($desiredColor[0],$desiredColor[1],$desiredColor[2]);

send_to_client($stringOutput);

	

?>