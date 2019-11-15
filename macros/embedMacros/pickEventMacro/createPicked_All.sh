#!/bin/bash
#Add Vertex file here
file=VertexLocation.txt
#${VMCWORKDIR}/parameters/VertexLocation.txt

i=0
declare -a runList
while IFS=$'\t' read -r -a myArray; do
    [[ "${myArray[0]}" =~ ^#.*$ ]] && continue
    runList[i]=${myArray[0]}
    ((i++))
done < "$file"
unique_runs=($(echo "${runList[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' '))

for i in "${unique_runs[@]}"
do
    . createPickedList.sh "${i}"
    source ${VMCWORKDIR}/build/config.sh
    root -b -q pickEvents.C\(${i}\)
done

for i in "${unique_runs[@]}"
do
    #cp generateMetadata.C ./picked/
    cd ./picked/
    RUN=${i} root -b -q generateMetadata.C
done

