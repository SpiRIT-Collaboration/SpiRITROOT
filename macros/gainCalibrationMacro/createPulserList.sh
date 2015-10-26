#!/bin/bash

##########################################################################################
# Configuration part #####################################################################
##########################################################################################

# Output list file
#   If the same file exists, existing file will be deleted.
OUTPUT=list.txt

# Voltage list separated by a space
#   Voltage order and the number of voltages should be the same as the file number order.
VOLTAGES=(5 4 3 2 1)

# Start and end number of the data
#   It'll loop over the numbers in two numbers
FIRST=0;
SECOND=4;

##########################################################################################
# Don't edit the below ###################################################################
##########################################################################################

> $OUTPUT

for i in $(seq $FIRST $SECOND);
do
  echo ${VOLTAGES[$((i-FIRST))]} >> $PWD/$OUTPUT
  if [ $i -le 1000 ];
  then
    ls /data/spdaq0?/narval/cobo??/run_0$i* >> $PWD/$OUTPUT
  else
    ls /data/spdaq0?/narval/cobo??/run_$i* >> $PWD/$OUTPUT
  fi
done
