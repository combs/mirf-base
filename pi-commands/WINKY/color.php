<?php


// Standard includes




function hex2RGB($hex) {
   $hex = str_replace("#", "", $hex);

   if(strlen($hex) == 3) {
      $r = hexdec(substr($hex,0,1).substr($hex,0,1));
      $g = hexdec(substr($hex,1,1).substr($hex,1,1));
      $b = hexdec(substr($hex,2,1).substr($hex,2,1));
   } else {
      $r = hexdec(substr($hex,0,2));
      $g = hexdec(substr($hex,2,2));
      $b = hexdec(substr($hex,4,2));
   }
   $rgb = array($r, $g, $b);
   //return implode(",", $rgb); // returns the rgb values separated by commas
   return $rgb; // returns an array with the rgb values
}

function encodeColorChannel($value) {
	$value=$value >> 2;
	$value+=32;
	return chr($value);
}

function encodeColor($red,$green,$blue) {
	$text="";
	$text .= encodeColorChannel($red );
	$text .= encodeColorChannel($green ); 
	$text .= encodeColorChannel($blue);	
	return $text;
}
function random_color_part() {
    return str_pad( dechex( mt_rand( 0, 255 ) ), 2, '0', STR_PAD_LEFT);
}

function random_color() {
    return random_color_part() . random_color_part() . random_color_part();
}


	
$modes=array(
	"C"=>"Color cycle",
	"f"=>"Fade to one color",
	"F"=>"Fade to one color (slow)",
	"p"=>"Pulse between two colors",
	"c"=>"Set to one color immediately",
	"i"=>"Idle animation (ignored unless nothing else has happened)"
	);
	
$speed_modes=array("C");


 
		

$data_dir="data";

$images_dir="posters";

$id="";
$savetext="";


if (!empty($_GET["m"])) {
	$id=$_GET["m"];
}  

$speed=isset($_GET['speed']) ? $_GET['speed'] : 3;
$mode=isset($_GET['mode']) ? $_GET['mode'] : "C";
 
	
if (isset($_GET['random']) || strpos($argv[0], "random") !== false ) { 
	$mode=array_keys($modes)[ rand(0, count($modes) - 2 ) ] ;  // hack -- ignore idle
	$speed=rand(1,9);
	
	for ($a=0;$a<8;$a++) {
		$_GET['color' . $a]="#ffffff";
	}
	for ($a=0;$a<rand(1,8);$a++) {
		$_GET['color' . $a]="#" . random_color();
	}
	

	
	
}


$command="WINKYBASES" . $mode;
if (in_array($mode,$speed_modes)) {
	$command .= $speed;
}

$gotone=false;
 
	for ($a=0;$a<8;$a++) {
		if (isset($_GET['color' . $a]) && $_GET['color' . $a] != "#ffffff") {
			
			$savetext.="\"" . $_GET['color' . $a] . "\",";
			
			$array=hex2RGB($_GET['color' . $a]); 
			$command .= encodeColor($array[0],$array[1],$array[2]);
			$gotone=true;
		}		
	}
	$savetext .= "\"#ffffff\"";
	$command .= "\n";
	
	if ($gotone===true) {
		file_put_contents("/var/local/nrf24/out/winky-web",$command);
	}
	
	if (isset($_GET['savename']) && strlen($_GET['savename']) > 0 ) {
		file_put_contents("data/" . filter_var($_GET['savename'] . ".txt", FILTER_SANITIZE_STRING), $savetext);
	}
	




echo("<!DOCTYPE html>
<html lang='en'>
  <head>
    <meta charset='utf-8'>
    <meta http-equiv='X-UA-Compatible' content='IE=edge'>

    <title> RAINBOW!! </title> <meta name='description' content='RAINBOW!!'> 
    <meta name='author' content='RAINBOW!! 🌈'>

    <!-- Enable responsive viewport -->
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>

    <!-- Bootstrap styles -->
    <link href='bootstrap/css/bootstrap.min.css' rel='stylesheet'>
 
     <meta property='og:title' content='" . htmlspecialchars($title) . "' />
<meta name='description' content='wow... " . htmlspecialchars("🌈  " . $plot) . "' />
    <meta name='og:description' content='wow? " . htmlspecialchars("🌈 " . $plot) . "' />
<meta name='keywords' content='wow' />

    <!-- Custom styles -->
    <link href='css/style.css' rel='stylesheet' type='text/css' media='all'>

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
      <script src='https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js'></script>
      <script src='https://oss.maxcdn.com/libs/respond.js/1.3.0/respond.min.js'></script>
    <![endif]-->

    <!-- Fav and touch icons -->
    <!-- Update these with your own images
      <link rel='shortcut icon' href='images/favicon.ico'>
      <link rel='apple-touch-icon' href='images/apple-touch-icon.png'>
      <link rel='apple-touch-icon' sizes='72x72' href='images/apple-touch-icon-72x72.png'>
      <link rel='apple-touch-icon' sizes='114x114' href='images/apple-touch-icon-114x114.png'>
    -->
 
	<!-- web fonts -->
	<link href='http://fonts.googleapis.com/css?family=Permanent+Marker|VT323' rel='stylesheet' type='text/css'>
	<script>
    function load() {
    	
    	$('input.color').data('touched','no');
    	$('input.color').val('#ffffff');
    	for (var a=0;a<arguments.length;a++) {
    	$('input.color').eq(a).val(arguments[a]);
    	$('input.color').eq(a).data('touched','yes');
    	}
    	
    }</script>
  </head>

  <body style=''>
	<!-- Google Tag Manager -->
	<noscript><iframe src='//www.googletagmanager.com/ns.html?id=GTM-NMBXLX'
	height='0' width='0' style='display:none;visibility:hidden'></iframe></noscript>
	<script>(function(w,d,s,l,i){w[l]=w[l]||[];w[l].push({'gtm.start':
	new Date().getTime(),event:'gtm.js'});var f=d.getElementsByTagName(s)[0],
	j=d.createElement(s),dl=l!='dataLayer'?'&l='+l:'';j.async=true;j.src=
	'//www.googletagmanager.com/gtm.js?id='+i+dl;f.parentNode.insertBefore(j,f);
	})(window,document,'script','dataLayer','GTM-NMBXLX');</script>
	<!-- End Google Tag Manager -->
	
	<div class='container'><div class='row text-center'> <h1 style='
	    color: white; 
    background: -moz-linear-gradient( top ,
        rgba(255, 0, 0, 1) 0%,
        rgba(255, 255, 0, 1) 15%,
        rgba(0, 255, 0, 1) 30%,
        rgba(0, 255, 255, 1) 50%,
        rgba(0, 0, 255, 1) 65%,
        rgba(255, 0, 255, 1) 80%,
        rgba(255, 0, 0, 1) 100%);
    background: -webkit-gradient(linear,  left top,  left bottom, 
        color-stop(0%, rgba(255, 0, 0, 1)), 
        color-stop(15%, rgba(255, 255, 0, 1)),
        color-stop(30%, rgba(0, 255, 0, 1)),
        color-stop(50%, rgba(0, 255, 255, 1)),
        color-stop(65%, rgba(0, 0, 255, 1)),
        color-stop(80%, rgba(255, 0, 255, 1)),
        color-stop(100%, rgba(255, 0, 0, 1)));
        
        ;padding:1em .5em;'>🌈🌈🌈 Every color hooray! 🌈🌈🌈</h1></div><div class='row'><div class='center-block main-movie-magic' style=''>
	<div class='panel panel-default'><div class='panel-heading text-center'> Pick it pick it </div><div class='panel-body text-center'><form action='color.php' method='GET'><input type='hidden' name='random' value='1'><input type='submit' value='✨🌈 RANDOM! 🌈✨'></form><br/><br/><form name='lizards' action='color.php' method='get'>Speed (for color cycling modes):<br/>Slow 
");
for ($b=1;$b<10;$b++) {
	echo("<input type='radio' name='speed' value='" . $b . "' " );
	echo( $speed==$b ? "checked" : "");
	echo("> " . $b . " ");
}
echo(" Fast<br/><br/><select name='mode'>");

foreach(array_keys($modes) as $key) {
	echo("<option value='" . $key . "'");
	if ($key == $mode) {
		echo(" selected ");
	}
	echo(">" . $modes[$key] . "</option>");	
}

echo("</select><br/><br/>");


	for ($a=0;$a<8;$a++) {
		echo("<input type='color' name='color" . $a . "' value='" );
		echo($_GET['color' . $a] ? $_GET['color' . $a] : '#ffffff');
		echo("' data-touched='" );
		echo( ( $_GET['color' . $a] && $_GET['color' . $a] != "#ffffff" )  ? "yes" : "no");
		echo("' class='color'>");
	}
	
 
 echo(" <br/>save it? filename: <input type='text' name='savename'>	<br/><br/><input type='submit' value='Do it!'></form><br/>load: ");
 
 
    
foreach(glob($data_dir . "/*.txt") as $thisone) {
 
	$filedata=file_get_contents($thisone);
	echo ("<a href='#' onClick='load(");
	echo($filedata . ")'>" . preg_replace('/\..*/','',basename($thisone)) . "</a> ");
}

 
 
 echo(" <br/><br/>command: <pre style='border: 2px solid grey'>" . $command . "</pre><br/><br/></div></div></div>
	</div>
	
    <script src='https://ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js'></script>
    <script src='bootstrap/js/bootstrap.min.js'></script>
    <script>$('input.color').click(function() { $(this).data('touched','yes');});
    $('form').submit(function() { $('input.color').each(function() { if ($(this).data('touched')!='yes') { $(this).remove();}; } );   return true;} ) ;
    
    
    </script>
    
  </body>
</html>


");



?>