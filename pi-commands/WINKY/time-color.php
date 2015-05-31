<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";
 

if ($currentMinute < 360 || $currentMinute > 1260) {
} else if ($currentMinute < 540) {	
	$desiredColor[0]=map($currentMinute,360,540,0,80);
	$desiredColor[1]=map($currentMinute,360,540,0,80);
	$desiredColor[2]=map($currentMinute,360,540,0,200);
} else if ($currentMinute < 840) {
	$desiredColor[0]=map($currentMinute,540,840,80,255);
	$desiredColor[1]=map($currentMinute,540,840,80,255);
	$desiredColor[2]=map($currentMinute,540,840,200,255);
} else if ($currentMinute < 1020) {
	$desiredColor[0]=map($currentMinute,840,1020,255,255);
	$desiredColor[1]=map($currentMinute,840,1020,255,120);
	$desiredColor[2]=map($currentMinute,840,1020,255,60);
} else {
		$desiredColor[0]=map($currentMinute,1020,1260,255,0);
	$desiredColor[1]=map($currentMinute,1020,1260,120,0);
	$desiredColor[2]=map($currentMinute,1020,1260,60,0); 
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