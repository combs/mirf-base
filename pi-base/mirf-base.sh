#!/bin/bash

INITIALIZED=false
BASE=/var/local/nrf24

while sleep 2; do
SERIAL=`ls -t /dev/tty.usb* /dev/ttyA* 2>/dev/null| grep -v ttyAMA0 | head -1`
echo "using $SERIAL" 
ln -sf $SERIAL /dev/mirf
if [ `uname` != "Darwin" ]
then

	stty -F $SERIAL cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts >/dev/null
else 
	stty -f $SERIAL cs8 115200 ignbrk -brkint   -echo -echoe -echok -echoctl >/dev/null

fi
 
exec 3<> $SERIAL 
sleep 1


while read TO FROM DATA DETAILS; do
	DATA=`echo $DATA|tr '\r' ' '|sed -e 's:[ \t]*$::'`
	DETAILS=`echo $DETAILS|tr '\r' ' '|sed -e 's:[ \t]*$::'`
 	WRITE=false
 	HANDLED=false
 	
 	if [ -e /tmp/mirf-debug ]
 		then
 			echo  "From $FROM to $TO, I got '$DATA' '$DETAILS'"
	fi
		
	if [ "$FROM" != "" ] 
		then
			 
			case "$FROM" in
				BASES)
					case "$TO" in
						BASES)
						case "$DATA" in
							UPDATE*)
							#	echo "Update requested"
								# do stuff;
								# echo -ne "."
								HANDLED=true
							;;
							Message_sent_to*)
								echo "Message successfully sent to $DETAILS."
								HANDLED=true
								;;
							Booted*)
								if [ "$DETAILS" = "14" ]
								then
								echo "Base station successfully started. Messages can now be sent."
								INITIALIZED=true
								else
								echo "Base station radio failed to start: status $DETAILS. "
								
								fi
								HANDLED=true
								;;
								
							*) 
						esac
						;;
		
						*) 
		
					esac
		
				;; 
				
				*)
				;;
			esac
			
		if [ "$HANDLED" != "true" ] 
		then
		 
		if [ -x "$BASE/commands/$FROM/$DATA" ]
			then
			echo "Calling $BASE/commands/$FROM/$DATA"
			
			$BASE/commands/$FROM/$DATA $FROM $DETAILS >&3 & 
		elif [ "$DATA" != "" -a -x "$BASE/commands/common/$DATA" ]
			then 
			echo "Calling $BASE/commands/common/$DATA"
			
			$BASE/commands/common/$DATA $FROM $DETAILS >&3 & 
		else
			WRITE=true
			
			
			# process fields here, and access them with $date, $time, etc.
			
		fi
		fi
			
			if [ "$WRITE" = "true" ]
			then
				echo  "From $FROM to $TO, I got $DATA"
				mkdir -p $BASE/in/`date +%Y-%m-%d`/$FROM/ && echo "$DATA $DETAILS" > "$BASE/in/`date +%Y-%m-%d`/$FROM/`date +%s.%N`"
				
			fi
		 
	
	fi
	if [ "$INITIALIZED" = "true" ]
	then
		OUTFILES=`find /var/local/nrf24/out -maxdepth 1 -type f `
		if [ "$OUTFILES" != "" ] 
			then
			
			for FILE in $OUTFILES
				do
				cat "$FILE" >&3 && rm "$FILE"
				done
		fi
		NAMEDOUTFILES=`find /var/local/nrf24/out -maxdepth 2 -mindepth 2 -type f`
		if [ "$NAMEDOUTFILES" != "" ] 
			then
			
			for FILE in $NAMEDOUTFILES
				do
				echo -n "`dirname $FILE | sed s:.*/::`BASES" >&3 && cat "$FILE" >&3 && rm "$FILE"
				done
		fi
	else 
		echo "Not checking for messages... isn't running."
	fi
	
	# sleep 0.25

done <&3 || INITIALIZED=false && echo "Error contacting base station. Message transmission disabled."


# look for things to send


exec 3>&-


done

