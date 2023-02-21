#!/bin/bash

# check if the filename argument is present
if [ $# -ne 1 ]; then
  echo "Usage: $0 filename.tig"
  exit 1
fi

# store the filename argument in a variable
filename="$1"

# check if the input file exists
if [ ! -f "$filename" ]; then
  echo "File not found: $filename"
  exit 1
fi

# run the dtiger program with the filename argument
../src/driver/dtiger -i --dump-ir "$filename"
echo |../src/driver/dtiger -i --dump-ir "$filename" > $filename.ll
