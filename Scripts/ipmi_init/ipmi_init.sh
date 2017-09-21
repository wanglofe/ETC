#!/bin/bash +x

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
IP=$1
CHANNEL_1=1
CHANNEL_8=8
PRIV=4

if [ $# -ne 1 ]
then
	echo "Error arguments ! "
	echo "Usage:    $0  <host-ip-address>"
	echo 
	exit 1
fi

# Enable RMCP
ipmitool -I lanplus -H $IP -U admin -P admin raw 0x3a 0x83 0x10 0x00 >/dev/null
 
# Create a new user 
ipmitool -I lanplus -H $IP -U admin -P admin user set name $USERID $USERNAME
ipmitool -I lanplus -H $IP -U admin -P admin user set password $USERID $PASSWORD 
ipmitool -I lanplus -H $IP -U admin -P admin user priv $USERID $PRIV $CHANNEL_1 
ipmitool -I lanplus -H $IP -U admin -P admin user priv $USERID $PRIV $CHANNEL_8
ipmitool -I lanplus -H $IP -U admin -P admin channel setaccess $CHANNEL_1 $USERID ipmi=on
ipmitool -I lanplus -H $IP -U admin -P admin channel setaccess $CHANNEL_8 $USERID ipmi=on 
ipmitool -I lanplus -H $IP -U admin -P admin user enable $USERID

echo
echo ----------------------------User List--------------------------------
ipmitool -I lanplus -H $IP -U admin -P admin user list

echo 
echo ----------------ipmi-chassis --get-chassis-status--------------------
ipmi-chassis -h $IP -u rootuser -p rootuser --get-chassis-status
