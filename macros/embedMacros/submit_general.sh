#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem-per-cpu=2000
#SBATCH -o "./log/slurm_%A_.log"
source ${VMCWORKDIR}/build/config.sh

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
root embedMacros/run_general.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > ${LOGDIR}_run.log


