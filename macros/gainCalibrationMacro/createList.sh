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
printf -v RUN "%04g" $RUN

OUTPUT=list_run$RUN\.txt
> $PWD/$OUTPUT

echo "== Creating list as $OUTPUT. Please be patient."

for COBO in $(seq -f "%02g" 0 11);
do
  ls /data/spdaq0?/narval/cobo$COBO/run_$RUN*s >> $OUTPUT
  for FILENUM in {1..100};
  do
    RESULT=`ls /data/spdaq0?/narval/cobo$COBO/run_$RUN*s.$FILENUM 2> /dev/null`
    if [[ -a $RESULT ]];
    then
      echo $RESULT >> $OUTPUT
    else
      break
    fi
  done
done

echo "== Done! Enjoy analysis!"
