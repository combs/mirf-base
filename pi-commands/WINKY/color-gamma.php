<?php

// Standard includes

$our_include_path=dirname(__FILE__) . ":" . dirname(__FILE__) . "/../common" . ":" . GETENV("HOME");
set_include_path($our_include_path);
require "get-directories.php";

$currentHour=date('H');
$currentMinute=date('i') + $currentHour*60;
$desiredColor=array(0,0,0);
$nameClient="WINKY";

$stringOutput="f";
$stringOutput.=encodeColor($details[0]*.8);
$stringOutput.=encodeColor($details[1]);
$stringOutput.=encodeColor($details[2]*.6);
send_to_client($stringOutput);

?>