#!/bin/bash
# int-or-string.sh

dataFile="data/MC_alpha_dEdx.dat"

rm $dataFile

for (( momentum=10; momentum<=2000; momentum+=50 ))
do
  root -q -l 'energyLossStudy.C("'$dataFile'", kTRUE, 100, 4, '$momentum')'
done

root -l plotAll.C
