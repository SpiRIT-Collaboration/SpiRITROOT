#!/bin/bash

##########################################################################################
# Don't edit the below ###################################################################
##########################################################################################

if [ -z "$1" ];
then
  echo "Please put the run number after the script file."
  echo "  ex) ./createList.sh 370"

  exit 1
fi

RUN=$1

if [ $RUN -le 1000 ];
then
  OUTPUT=list_run0$RUN\.txt
  > $PWD/$OUTPUT
  ls /data/spdaq0?/narval/cobo??/run_0$RUN* >> $PWD/$OUTPUT
else
  OUTPUT=list_run$RUN\.txt
  > $PWD/$OUTPUT
  ls /data/spdaq0?/narval/cobo??/run_$RUN* >> $PWD/$OUTPUT
fi
