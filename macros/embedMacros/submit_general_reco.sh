#!/usr/bin/env bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=6
#SBATCH --mem-per-cpu=2000
#SBATCH --array=0-18
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_%a.log"
source ${VMCWORKDIR}/build/config.sh

cd ${VMCWORKDIR}\/macros

OUTPUT=$1
NEXTJOB=$2
CONFIGLIST=$3
MCFILE=$4

# Creat directory for logging 
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros

RUN=2841
NTOTAL=10000
NSPLIT=200
GCData=/mnt/spirit/rawdata/misc/gainCalibration_groundPlane_120fC_117ns_20160509.root
GGData=/mnt/spirit/rawdata/misc/ggNoise/ggNoise_2842.root
MCFile=${VMCWORKDIR}/macros/data/${MCFILE}.digi.root
#MCFile=
SupplePath=/mnt/spirit/rawdata/misc/picked_tommy/
ParameterFile=ST.parameters.PhysicsRuns_201707.par
threshold=0.5

echo $OUTPUT
echo ${SLURM_ARRAY_TASK_ID}

SPLIT=$((3*SLURM_ARRAY_TASK_ID+0)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+1)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+2)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &

echo root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE,${threshold}\) -b -q -l

wait

cd ${VMCWORKDIR}/macros
if (( SLURM_ARRAY_TASK_ID == 0 )); then
    root ${VMCWORKDIR}\/macros\/embedMacros/run_general_reco.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > ${LOGDIR}_run.log
fi


