#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem-per-cpu=2000
#SBATCH -o "./log/slurm_%A.log"
source ${VMCWORKDIR}/build/config.sh

OUTPUT=$1
NEXTJOB=$2
CONFIGLIST=$3
MCFILE=$4

# Creat directory for logging 
LOGDIR=log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros

RUN=2841
NTOTAL=10000
NSPLIT=10000
GCData=/mnt/spirit/rawdata/misc/gainCalibration_groundPlane_120fC_117ns_20160509.root
GGData=/mnt/spirit/rawdata/misc/ggNoise/ggNoise_2842.root
#MCFile=/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data/${MCFILE}
#MCFile=
SupplePath=/mnt/spirit/rawdata/misc/picked_tommy/
ParameterFile=ST.parameters.PhysicsRuns_201707.par
threshold=0.5

echo $OUTPUT
echo ${SLURM_ARRAY_TASK_ID}

SPLIT=0; root run_reco_mc.C\(\"\",$RUN,$NTOTAL,$SPLIT,$NSPLIT,30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\"\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &

echo root run_reco_mc.C\(\"\",$RUN,$NTOTAL,$SPLIT,$NSPLIT,30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\"\) -b -q -l

wait

cd ${VMCWORKDIR}/macros
if (( SLURM_ARRAY_TASK_ID == 0 )); then
    root embedMacros/run_general_mc.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > ${LOGDIR}_run.log
fi


