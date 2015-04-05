#!/bin/bash
cd `dirname $0`
. ../common/get-directories

cd "$BASEDIR/binaries/GIFFR" || exit;
 

SERIAL=`ls -t /dev/tty.usb* /dev/ttyA* 2>/dev/null| grep -v ttyAMA0 | head -1`
echo "using $SERIAL" 
if [ `uname` != "Darwin" ]
then

	stty -F $SERIAL cs8 57600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts >/dev/null
else 
	stty -f $SERIAL cs8 57600 cread -clocal -crtscts >/dev/null

fi

echo "opening $SERIAL"

exec 3<> $SERIAL 
 
echo "opened $SERIAL"
sleep 3

for arg in *.raw; do echo "uuencoding $arg";uuencode "$arg" "$arg" > "`echo \"$arg\"|sed -e 's:raw:uue:'`" && mv "$arg" "/tmp/$arg"; done

rm '*.uue' 2>/dev/null

for arg in *.uue; do echo "sending $arg" ; head -1 "$arg" >&3; 
sleep 1; 
sed -e '1d' "$arg" | pv -L 4k >&3
	DONE=""
	while true;
	do read DONE <&3 ;
	echo "$DONE" | grep next && break;
	echo $DONE
	done
#	sleep 5;
	mv "$arg" "/tmp/$arg"
if [ `uname` != "Darwin" ]
then

	stty -F $SERIAL cs8 57600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts >/dev/null
else 
	stty -f $SERIAL cs8 57600 cread -clocal -crtscts >/dev/null

fi
	sleep 3	
	
done


done



