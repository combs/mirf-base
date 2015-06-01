<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";


$times=array(0, 330, 420, 840, 1020, 1150, 1260, 1441);
$colors=array(  array(0,0,0),
			 	array(0,0,0),
			 	array(40,80,100),
			 	array(200,255,180),
			 	array(255,120,60),
			 	array(0,0,60),
			 	array(0,0,0),
			 	array(0,0,0)
			 	);

$count=count($times);

for($i=0; $i < ($count - 1); $i++) {
	if ( $currentMinute < $times[$i+1]   && 
		($currentMinute > $times[$i] ) 
		)  {
		$desiredColor[0]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][0],$colors[$i+1][0]);
		$desiredColor[1]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][1],$colors[$i+1][1]);
		$desiredColor[2]=map($currentMinute,$times[$i],$times[$i+1],$colors[$i][2],$colors[$i+1][2]);
	}
	
	
}

$stringOutput="f";
$stringOutput.=encodeColor($desiredColor[0]);
$stringOutput.=encodeColor($desiredColor[1]);
$stringOutput.=encodeColor($desiredColor[2]);
send_to_client($stringOutput);

	

?>