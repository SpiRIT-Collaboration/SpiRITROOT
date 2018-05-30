#!/bin/bash

export OMP_NUM_THREADS=10

mar18

SPROOTVER=SpiRITROOT.may24
SOFTDIR=/cache/scr/spirit/mizuki/$SPROOTVER
source $SOFTDIR/build/config.sh

cd $SOFTDIR/macros/

#NAME=UrQMD_300AMeV_short
NUMEVT=10
NAME=p_th0-85ph180_ta$NUMEVT
EVENT=${NAME}.root
USEFIELDMAP=kTRUE

if [ $RUNG4 == "yes" ]; then
    root -l -b -q run_mc.C\(\"$NAME\",$NUMEVT,$USEFIELDMAP\)

    root -l -b -q run_digi.C\(\"$NAME\"\)
fi

root -l -b -q run_reco_mc.C\(\"$NAME\",\"data\/\",0,$NUMEVT\)

