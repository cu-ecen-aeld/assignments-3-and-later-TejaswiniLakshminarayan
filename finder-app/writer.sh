#!/bin/sh

# writefile represents full path to a file on the filesystem
# writestr represents the text string which will be written within this file
writefile=$1
writestr=$2 

# check if any arguments were missing
if [ "$#" != "2" ];
then
	echo "Two arguments needs to be passed"
	exit 1
fi


# check if the directory exists
{ 
	if [ ! -d "$1" ];
	then
	# creating the path
		mkdir -p "$(dirname .$1)" && touch ".$1" && echo "$2" > ".$1"
	else
		echo "$2" > ".$1"
	fi

} || {
	echo "file could not be created"
	exit 1
}

	


