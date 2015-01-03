#!/bin/bash

INITIALIZED=false
BASE=/var/local/nrf24

while sleep 2; do
SERIAL=`ls -t /dev/tty.usb* /dev/ttyA* 2>/dev/null| grep -v ttyAMA0 | head -1`
echo "using $SERIAL" 
stty -F $SERIAL cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts >/dev/null
 
exec 3<> $SERIAL 
sleep 0.2


while read TO FROM DATA DETAILS; do
	DATA=`echo $DATA|tr '\r' ' '|sed -e 's:[ \t]*$::'`
	DETAILS=`echo $DETAILS|tr '\r' ' '|sed -e 's:[ \t]*$::'`
 	 echo  "From $FROM to $TO, I got '$DATA' '$DETAILS'"
	
	 if [ "$FROM" != "" ] 
		then
			
			WRITE=false
			
		if [ -x "$BASE/commands/$FROM/$DATA" ]
			then
			echo "Calling $BASE/commands/$FROM/$DATA"
			
			$BASE/commands/$FROM/$DATA $FROM $DETAILS >&3
		elif [ "$DATA" != "" -a -x "$BASE/commands/common/$DATA" ]
			then 
			echo "Calling $BASE/commands/common/$DATA"
			
			$BASE/commands/common/$DATA $FROM $DETAILS >&3
		else
			
			# process fields here, and access them with $date, $time, etc.
			case "$FROM" in
				BASES)
					case "$TO" in
						BASES)
						case "$DATA" in
							UPDATE*)
							#	echo "Update requested"
								# do stuff;
								# echo -ne "."
							;;
							Message_sent_to*)
								echo "Message successfully sent to $DETAILS."
								;;
							Booted*)
								if [ "$DETAILS" = "14" ]
								then
								echo "Base station successfully started. Messages can now be sent."
								INITIALIZED=true
								else
								echo "Base station radio failed to start: status $DETAILS. "
								fi
								sleep 3
								;;
								
							*)
								WRITE=true
						esac
						;;
		
						*)
							WRITE=true
		
					esac
		
				;;
				BUS)
		
				;;
				WCLKK)
					case "$TO" in
						BASES)
						case "$DATA" in
							update*)
								echo "WCLKKBASES0Today's weather:" >&3
								sleep 0.1;
								echo "WCLKKBASES1Byoootiful" >&3
								# do stuff;
							;;
							*)
								WRITE=true
						esac
						;;
		
						*)
							WRITE=true
		
					esac
		
				;;
		
				SNCLK)
		
				;;
				*)
				WRITE=true;;
			esac
			if [ "$WRITE" = "true" ]
			then
				echo  "From $FROM to $TO, I got $DATA"
				mkdir -p $BASE/in/$FROM/ && echo "$DATA $DETAILS" > "$BASE/in/$FROM/`date +%s`"
				
			fi
		
		fi
	
	fi
	if [ "$INITIALIZED" = "true" ]
	then
		
		if [ `find /var/local/nrf24/out -maxdepth 1 -type f | grep -c '$'` -gt 0 ] 
			then
			
			for FILE in `find /var/local/nrf24/out -maxdepth 1 -type f`
				do
				cat "$FILE" && cat "$FILE" >&3 && rm "$FILE" && sleep 0.5
				done
		fi
		if [ `find /var/local/nrf24/out -maxdepth 2 -mindepth 2 -type f | grep -c '$'` -gt 0 ] 
			then
			
			for FILE in `find /var/local/nrf24/out -maxdepth 2 -mindepth 2 -type f`
				do
				cat "$FILE" && echo -n "`dirname $FILE | sed s:.*/::`BASES" >&3 && cat "$FILE" >&3 && rm "$FILE" && sleep 0.5
				done
		fi
	fi
	
	# sleep 0.25

done <&3 || INITIALIZED=false && echo "Error contacting base station. Message transmission disabled."


# look for things to send


sleep 0.25
exec 3>&-


done

