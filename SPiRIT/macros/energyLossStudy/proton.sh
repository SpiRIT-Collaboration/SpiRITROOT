#!/bin/bash
# int-or-string.sh

dataFile="data/energyLossProton.dat"

rm $dataFile

for (( momentum=10; momentum<=400; momentum+=10 ))
do
  root -q -l 'energyLossStudy.cc("'$dataFile'", kTRUE, 10, 0, '$momentum')'
done

root -l 'plotEnergyLoss.cc("'$dataFile'")'
