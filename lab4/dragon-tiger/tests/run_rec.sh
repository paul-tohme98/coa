#!/bin/bash

# check if a directory argument was provided
if [ -z "$1" ]; then
  echo "Usage: $0 directory"
  exit 1
fi

# set the directory to loop through
directory="$1"

# check if the directory exists
if [ ! -d "$directory" ]; then
  echo "Error: $directory does not exist or is not a directory"
  exit 1
fi

# loop through each file in the directory
for f in "$directory"/*.tig; do
	# check if the file exists
  	if [ -e "$f" ]; then
		# run the dtiger program with the filename argument
    		../src/driver/dtiger -i --dump-ir "$f"
		echo | ../src/driver/dtiger -i --dump-ir "$f" > $f.ll
	fi
done
