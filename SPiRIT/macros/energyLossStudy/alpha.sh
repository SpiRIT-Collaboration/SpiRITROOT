#!/bin/bash
# int-or-string.sh

dataFile="data/energyLossAlpha.dat"

rm $dataFile

for (( momentum=10; momentum<=1000; momentum+=20 ))
do
  root -q -l 'energyLossStudy.cc("'$dataFile'", kFALSE, 10, 4, '$momentum')'
done

root -l plotAll.C
