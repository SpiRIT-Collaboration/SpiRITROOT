#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --array=0-9
#SBATCH --mem=3000
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_SARR_%a_MCDigi.log"
#SBATCH --time=05:00:00
#SBATCH --job-name="MC"

module load GCCcore/6.4.0
source ${VMCWORKDIR}/build/config.sh
export LD_LIBRARY_PATH=${VMCWORKDIR}/macros/lib:$LD_LIBRARY_PATH

OUTPUT=$1
CONFIGLIST=$2
CONFIGFILE=$3
NTOTAL=$(($4/10))

OUTPUT=${OUTPUT}_SARR_${SLURM_ARRAY_TASK_ID}

# Creat directory for logging 
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros
SPLIT=$((NTOTAL*SLURM_ARRAY_TASK_ID))
root run_mc.C\(\"$OUTPUT\",${NTOTAL},\"\",\"data/\",kTRUE,\"$CONFIGFILE\",$SPLIT\) -b -q -l > ${LOGDIR}_mc.log
sleep 1
root run_digi.C\(\"$OUTPUT\"\) -b -q -l > ${LOGDIR}_digi.log

