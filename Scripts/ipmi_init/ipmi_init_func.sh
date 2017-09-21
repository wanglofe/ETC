#!/bin/bash

echo
echo ---------------------------------------------------------------------
echo -e '\t' 'Create a new Administrator user with < ID: 3 > in user list'
echo -e '\t' 'Username/Password :rootuser/rootuser'
echo -e '\t' 'Use this account do IPMI-chassis operations'
echo ---------------------------------------------------------------------
echo

USERID=3
USERNAME='rootuser'
PASSWORD='rootuser'
IP=$1
CHANNEL=1
PRIV=4

if [ $# -ne 1 ]
then
	echo "Error arguments ! "
	echo "Usage: $0     <host-ip-address>"
	echo 
	exit 1
fi

# IPMI function
function IPMI
{
	ipmitool -I lanplus -H $IP -U admin -P admin $*	
}

# Enable RMCP
IPMI raw 0x3a 0x83 0x10 0x00 > /dev/null
 
# Create a new user 
IPMI user set name $USERID $USERNAME
IPMI user set password $USERID $PASSWORD 
IPMI user priv $USERID $PRIV $CHANNEL 
IPMI channel setaccess $CHANNEL $USERID callin=on ipmi=on link=on privilege=$PRIV
IPMI user enable $USERID

echo
echo ----------------------------User List--------------------------------
IPMI user list

# Example process
echo
echo -------------------IPMI-chassis --get-chassis-status-----------------
ipmi-chassis -h $IP -u rootuser -p rootuser --get-chassis-status  
