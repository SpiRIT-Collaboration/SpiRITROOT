#!/bin/bash
#Add Vertex file here
file=${VMCWORKDIR}/parameters/VertexLocation.txt

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
done

