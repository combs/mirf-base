#!/bin/bash

while sleep 5; do
SERIAL=`ls -t /dev/tty.usb* /dev/ttyA* 2>/dev/null| grep -v ttyAMA0 | head -1`
echo "using $SERIAL" 
stty -F $SERIAL cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts >/dev/null



while read TO FROM DATA; do
# 	echo  "From $FROM to $TO, I got $DATA"


	if [ -e "/var/local/nrf24/commands/$FROM/$DATA" ]
	then
	/var/local/commands/$FROM/$DATA $SERIAL
	fi
	
	WRITE=false
	if [ "$FROM" != "" ] 
		then

		# process fields here, and access them with $date, $time, etc.
		echo  "From $FROM to $TO, I got $DATA"
		case "$FROM" in
			BASES)
				case "$TO" in
					BASES)
					case "$DATA" in
						UPDATE*)
							echo "Update requested"
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
			BUS)
	
			;;
			WCLKK)
				case "$TO" in
					BASES)
					case "$DATA" in
						update*)
							echo "WCLKKBASES0Today's weather:" > $SERIAL
							echo "WCLKKBASES1Byoootiful" > $SERIAL
							# do stuff;
							break
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
			echo "writing"
		fi
	

	fi

done < $SERIAL


# look for things to send


sleep 0.25

done

