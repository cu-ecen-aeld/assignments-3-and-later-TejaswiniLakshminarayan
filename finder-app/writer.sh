#!/bin/sh

writefile=$1
writestr=$2 

if [ "$#" != "2" ];
then
	echo "Two arguments needs to be passed"
	exit 1
else
	if [ ! -d "$1" ];
	then
		mkdir -p "$(dirname .$1)" && touch ".$1" && echo "$2" > ".$1"
		
	else
		echo "$2" > ".$1"
	fi
fi
