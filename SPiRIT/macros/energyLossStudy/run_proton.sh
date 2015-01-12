#!/bin/bash
# int-or-string.sh

dataFile="data/MC_proton_dEdx.dat"

rm $dataFile

for (( momentum=10; momentum<=1000; momentum+=20 ))
do
  root -q -l 'energyLossStudy.C("'$dataFile'", kTRUE, 100, 0, '$momentum')'
done

root -l plotAll.C
