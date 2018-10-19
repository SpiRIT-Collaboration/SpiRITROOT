#!/usr/bin/env bash
#SBATCH -o "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/log/slurm_%A.log"
VMCWORKDIR=/mnt/spirit/analysis/user/tsangc/SpiRITROOT
source $VMCWORKDIR/build/config.sh

OUTPUT=$1
NEXTJOB=$2
CONFIGLIST=$3
CONFIGFILE=$4

# Creat directory for logging 
LOGDIR=log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros
root run_mc.C\(\"$OUTPUT\",-1,\"\",\"data/\",kTRUE,\"$CONFIGFILE\"\) -b -q -l > ${LOGDIR}_mc.log
root run_digi.C\(\"$OUTPUT\"\) -b -q -l > ${LOGDIR}_digi.log
root run_general.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > ${LOGDIR}_run.log 
