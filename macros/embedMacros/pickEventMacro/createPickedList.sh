#!/bin/bash

##########################################################################################
# Don't edit the below ###################################################################
##########################################################################################

if [ -z "$1" ];
then
  echo "== Please put the run number after the script file."
  echo "==   ex) ./createPickedList.sh 370"

  exit 1
fi

RUN=$1
printf -v RUN "%04g" $RUN

if [[ ! -d "$PWD/picked/run_${RUN}" ]]
then
  mkdir -p picked/run_${RUN}
fi

OUTPUT=dataList.txt
> picked/run_${RUN}/$OUTPUT

echo "== Creating list as $OUTPUT of run ${RUN}. Please be patient."

for COBO in $(seq -f "%02g" 0 11);
do
    echo "${VMCWORKDIR}/macros/embedMacros/pickEventMacro/picked/run_${RUN}/pickedData/run${RUN}_cobo${COBO}" >> picked/run_${RUN}/$OUTPUT
done

echo "== Done! Enjoy picking and analysis!"
