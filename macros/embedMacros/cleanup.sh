#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1000
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_Cleanup.log"
#SBATCH --time=02:00:00

OUTPUT=$1
CONFIGLIST=$2
NEXTJOB=$3
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT} 

module load GCCcore/6.4.0
source ${VMCWORKDIR}/build/config.sh
export LD_LIBRARY_PATH=${VMCWORKDIR}/macros/lib:$LD_LIBRARY_PATH


root ${VMCWORKDIR}\/macros\/embedMacros/run_general_all.C\(\"${CONFIGLIST}\"\,${NEXTJOB},1\) -b -q -l > ${LOGDIR}_run.log
rm -r $VMCWORKDIR/macros/data/$OUTPUT
rm $VMCWORKDIR/macros/data/$OUTPUT_SARR*.digi.root
rm $VMCWORKDIR/macros/data/$OUTPUT.digi.root
rm $VMCWORKDIR/macros/data/${OUTPUT}_SARR*.params.root
