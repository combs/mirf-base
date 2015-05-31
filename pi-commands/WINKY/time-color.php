<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";


$times=array(0, 360, 540, 840, 1020, 1150, 1260, 1441);
$colors=array(  array(0,0,0),
			 	array(0,0,0),
			 	array(80,80,200),
			 	array(255,255,255),
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



$desiredColor[0]=$desiredColor[0] >> 2;
$desiredColor[1]=$desiredColor[1] >> 2;
$desiredColor[2]=$desiredColor[2] >> 2;
$desiredColor[0]+=32;
$desiredColor[1]+=32;
$desiredColor[2]+=32;

$stringOutput="c";
$stringOutput.=chr($desiredColor[0]);
$stringOutput.=chr($desiredColor[1]);
$stringOutput.=chr($desiredColor[2]);
send_to_client($stringOutput);


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
	
	

?>