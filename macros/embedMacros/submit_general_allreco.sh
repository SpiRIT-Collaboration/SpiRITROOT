#!/usr/bin/env bash
#SPATCH --ntask=1
#SBATCH --cpus-per-task=3
#SBATCH --mem=12000
#SBATCH --array=0-16
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_%a.log"
#SBATCH --time=05:00:00
#SBATCH --job-name="Reco"

module load GCCcore/6.4.0
source ${VMCWORKDIR}/build/config.sh
export LD_LIBRARY_PATH=${VMCWORKDIR}/macros/lib:$LD_LIBRARY_PATH


cd ${VMCWORKDIR}\/macros

OUTPUT=$1
NEXTJOB=$2
CONFIGLIST=$3
MCFILE=$4
DESTINATION=$5
FISHTANKUSER=$6
NTOTAL=$7

# Creat directory for logging 
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros

RUN=2841
let NSPLIT=($NTOTAL+3*17-1)/3/17
GCData=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/gainCalibration_groundPlane_120fC_117ns_20160509.root
GGData=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/ggNoise_2842.root
MCFile=${VMCWORKDIR}/macros/data/${MCFILE}.digi.root
#MCFile=
SupplePath=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/
ParameterFile=ST.parameters.PhysicsRuns_201707.par

SPLIT=$((3*SLURM_ARRAY_TASK_ID+0)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SPLIT}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+1)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SPLIT}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+2)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SPLIT}_reco.log  &


wait
#ssh -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $FISHTANKUSER@fishtank "mkdir -p ${DESTINATION}/${OUTPUT}"
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+0)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION/$OUTPUT/
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+1)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION/$OUTPUT/
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+2)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION/$OUTPUT/


#cd ${VMCWORKDIR}/macros
#if (( SLURM_ARRAY_TASK_ID == 0 )); then
#    root ${VMCWORKDIR}\/macros\/embedMacros/run_general_all.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -b -q -l > ${LOGDIR}_run.log
#fi


