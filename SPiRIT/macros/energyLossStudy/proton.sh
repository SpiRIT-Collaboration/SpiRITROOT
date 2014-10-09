#!/bin/bash
# int-or-string.sh

rm data/energyLossProton.dat

for (( momentum=10; momentum<=400; momentum+=10 ))
do
  root -q -l 'energyLossStudy.cc("data/energyLossProton.dat", kFALSE, 10, 0, '$momentum')'
done

root -l 'plotEnergyLoss.cc("data/energyLossProton.dat")'
