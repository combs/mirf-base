#!/bin/bash
cd `dirname $0`
. ../common/get-directories

cd "$BASE/binaries/GIFFR" || exit;
 

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

for arg in *.uue; do echo "sending $arg" ; head -1 "$arg" > &3; sleep 1; sed -e /0/d "$arg" >&3

	DONE=""
	while [ "$DONE" != "next" ];  do
	read DONE <&3 ;
	done
	mv "$arg" "/tmp/$arg"
	stty -f $SERIAL cs8 57600 ignbrk -brkint   -echo -echoe -echok -echoctl >/dev/null
	sleep 3	
	
done


done



