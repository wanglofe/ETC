#!/bin/bash  

echo
echo ---------------------------------------------------------------------
echo -e '\t' 'Open ipmi v1.5 RMCP, make lan channel available'
echo -e '\t' 'Create a new Administrator user with < ID: 15 > in user list'
echo -e '\t' 'Username/Password :rootuser/rootuser'
echo ---------------------------------------------------------------------
echo

USERID=15
USERNAME='rootuser'
PASSWORD='rootuser'
IP=$2
MODE=$1
CHANNEL_1=1
CHANNEL_8=8
PRIV=4

# Usage func
function Usage
{
	echo -e '\t'"Usage:"
	echo -e '\t'"Help mode  : $0   --help"
	echo -e '\t'"Client mode: $0   --clientmode  <host-ip-address>"
	echo -e '\t'"Host mode  : $0   --hostmode"
	echo
}

# IPMI function
function IPMI
{	
	if [ $MODE = '--hostmode' ];then 
		ipmitool $*
	elif [ $MODE = '--clientmode' ];then
		ipmitool -I lanplus -H $IP -U admin -P admin $*	
	else
		Usage
		exit 1
	fi
}

function IPMICHASSIS
{
	if [ $MODE = '--hostmode' ];then 
		ipmi-chassis $*
	elif [ $MODE = '--clientmode' ];then
		ipmi-chassis -h $IP -u admin -p admin $*	
	else
		Usage
		exit 1
	fi
	
}

if [ $# = 0 ] || [ $1 = '--help' ] || [ $1 = '-h' ]
then
	Usage
	exit 1
fi

# Enable RMCP
IPMI raw 0x3a 0x83 0x10 0x00 > /dev/null
 
# Create a new user 
IPMI user set name $USERID $USERNAME
IPMI user set password $USERID $PASSWORD 
IPMI user priv $USERID $PRIV $CHANNEL_1
IPMI user priv $USERID $PRIV $CHANNEL_8 
IPMI channel setaccess $CHANNEL_1 $USERID ipmi=on
IPMI channel setaccess $CHANNEL_8 $USERID ipmi=on
IPMI user enable $USERID

echo
echo ----------------------------User List--------------------------------
IPMI user list

# Example process
echo
echo -------------------IPMI-chassis --get-chassis-status-----------------
IPMICHASSIS --get-chassis-status  
