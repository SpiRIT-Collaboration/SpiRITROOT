#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=3000
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_.log"
#SBATCH --time=05:00:00
module load GCCcore/6.4.0
source ${VMCWORKDIR}/build/config.sh
export LD_LIBRARY_PATH=${VMCWORKDIR}/macros/lib:$LD_LIBRARY_PATH

OUTPUT=$1
CONFIGLIST=$2
CONFIGFILE=$3

# Creat directory for logging 
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros
root run_mc.C\(\"$OUTPUT\",-1,\"\",\"data/\",kTRUE,\"$CONFIGFILE\"\) -b -q -l > ${LOGDIR}_mc.log
root run_digi.C\(\"$OUTPUT\"\) -b -q -l > ${LOGDIR}_digi.log

#remove all mc files
rm $VMCWORKDIR/macros/data/$OUTPUT.mc.root

