#!/bin/bash  

echo ---------------------------------------------------------------------
echo -e "\t Open ipmi v1.5 RMCP, make lan channel available"
echo -e "\t Create a new Administrator user with < ID: 15 > in user list"
echo -e "\t Username/Password :rootuser/rootuser"
echo ---------------------------------------------------------------------

USERID=15
USERNAME='rootuser'
PASSWORD='rootuser'
IP=$2
MODE=$1
CHANNEL_1=1
CHANNEL_8=8
PRIV=4

show_usage(){
	echo -e "\tUsage: $0    <Options>"
	echo -e "\tOptions:"
	echo -e "\t\tHelp mode  : --help"
	echo -e "\t\tPC mode    : --pc-mode  <host-ip-address>"
	echo -e "\t\tHost mode  : --host-mode"
	echo
}

IPMI() {	
	case $MODE in
	'--host-mode')
		ipmitool $*;;
	'--pc-mode')
		ipmitool -I lanplus -H $IP -U admin -P admin $*;;
	esac
}

IPMICHASSIS(){
	case $MODE in
	'--host-mode')
		ipmi-chassis $*;;
	'--pc-mode')
		ipmi-chassis -h $IP -u admin -p admin $*;;
	esac
}

if [ $# = 0 ] || [ $1 = '--help' ] 
then
	show_usage
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
