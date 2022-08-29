#!/bin/sh

# filesdir represents the path to a directory on the filesystem
# searchstr represents a text string which will be searched within these files
filesdir=$1
searchstr=$2 

# Check if number of arguments is equal to 2
if [ "$#" != "2" ];
then
	echo "Two arguments needs to be passed"
	exit 1
fi

# Check if filesdir represent a valid directory
if [ ! -d "$1" ];
then
	echo "$filesdir does not represent a directory on a file system"
	exit 1
fi

# num_files represents number of files having the text
# num_lines represents number of matching lines
num_files=$(grep -lr "$2" ".$1" | wc -l)
num_lines=$(grep -r "$2" ".$1" | wc -l)
echo "The number of files are $num_files and the number of matching lines are $num_lines"







