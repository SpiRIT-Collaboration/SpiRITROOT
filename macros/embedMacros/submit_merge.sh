#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=10
#SBATCH --mem-per-cpu=3000
#SBATCH --time=02:00:00
#SBATCH --job-name="MERGE"

module load GCCcore/6.4.0
source ${VMCWORKDIR}/build/config.sh
export LD_LIBRARY_PATH=${VMCWORKDIR}/macros/lib:$LD_LIBRARY_PATH

OUTPUT=$1

hadd -j 10 $VMCWORKDIR/macros/data/${OUTPUT}.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_0.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_1.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_2.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_3.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_4.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_5.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_6.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_7.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_8.digi.root $VMCWORKDIR/macros/data/${OUTPUT}_SARR_9.digi.root
