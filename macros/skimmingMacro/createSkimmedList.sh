#!/bin/bash

##########################################################################################
# Don't edit the below ###################################################################
##########################################################################################

if [ -z "$1" ];
then
  echo "== Please put the run number after the script file."
  echo "==   ex) ./createSKimmedList.sh 370"

  exit 1
fi

RUN=$1
printf -v RUN "%04g" $RUN

OUTPUT=list_run$RUN\_skimmed.txt
> $PWD/$OUTPUT

if [[ ! -d "$PWD/skimmed" ]]
then
  mkdir $PWD/skimmed
fi

echo "== Creating list as $OUTPUT. Please be patient."

for COBO in $(seq -f "%02g" 0 11);
do
  echo "$PWD/skimmed/run_${RUN}_cobo${COBO}_skimmed" >> $OUTPUT
done

echo "== Done! Enjoy skimming and analysis!"
