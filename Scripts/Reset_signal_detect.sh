#!/bin/sh

echo "*************************************************"
echo "Usage: run this script when System is processing."
echo "*************************************************"
echo "Reset signal detection start ! "
echo 
echo 

lowstatus="Pin is Low  "

currentstatus=$(gpiotool 96 --get-data)
if [ "$lowstatus" != "$currentstatus" ]
then 
	while : 
	do      
		currentstatus=$(gpiotool 96 --get-data)
		if [ "$lowstatus" = "$currentstatus" ]
		then
                	echo $currentstatus,System reset signal is detected!
                	break
		else
			echo $currentstatus,detection is going on...
        	fi      
	done
else
	echo "Pin is in low-status now, wait for a moment and try again. "
fi
