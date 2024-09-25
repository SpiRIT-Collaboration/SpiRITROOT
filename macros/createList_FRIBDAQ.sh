#!/bin/bash

##########################################################################################
# Don't edit the below ###################################################################
##########################################################################################

if [ -z "$1" ];
then
  echo "== Please put the run number after the script file."
  echo "==   ex) ./createList.sh 370"

  exit 1
fi

RUN=$1
LOCATION=${HOSTNAME:0:4}
if [ $LOCATION == "span" ]
then
  DATAPATH=/data/spdaq04/data/fribdaq # SPANA01
elif [ $LOCATION == "ricc" ]
then
  DATAPATH=/data/Q15264/rawdata # RICC
else
  DATAPATH=/mnt/cephfs/hira/SPIRIT_2024/SPIRIT_TPC_data/fribdaq
fi

printf -v RUN "%04g" $RUN

OUTPUT=list_run$RUN\.txt
> $PWD/$OUTPUT

echo "== Creating list as $OUTPUT. Please be patient."

for COBO in $(seq -f "%02g" 0 11);
do
  ls $DATAPATH/cobo$COBO/*run-$RUN-00.evt >> $OUTPUT
  for FILENUM in {1..99};
  do
    printf -v MYFILENUM "%02g" $FILENUM
    RESULT=`ls $DATAPATH/cobo$COBO/*run-$RUN-$MYFILENUM.evt 2> /dev/null`
    if [[ -a $RESULT ]];
    then
      echo $RESULT >> $OUTPUT
    else
      break
    fi
  done
done

echo "== Done! Enjoy analysis!"
