#!/usr/bin/env bash
#SPATCH --ntask=1
#SBATCH --cpus-per-task=6
#SBATCH --mem-per-cpu=2000
#SBATCH --array=0-18
#SBATCH -o "LOGDIRTOBESUB/slurm_%A_%a.log"
#SBATCH --time=01:30:00
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

# Creat directory for logging 
LOGDIR=${VMCWORKDIR}\/macros\/log\/${OUTPUT}
mkdir -p "${LOGDIR%/*}"

cd $VMCWORKDIR/macros

RUN=2841
NTOTAL=1000
NSPLIT=20
GCData=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/gainCalibration_groundPlane_120fC_117ns_20160509.root
GGData=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/ggNoise_2842.root
MCFile=${VMCWORKDIR}/macros/data/${MCFILE}.digi.root
#MCFile=
SupplePath=/mnt/home/tsangchu/SpiRITROOT/Picked_run_2841/
ParameterFile=ST.parameters.PhysicsRuns_201707.par
threshold=0.5

echo $OUTPUT
echo ${SLURM_ARRAY_TASK_ID}

SPLIT=$((3*SLURM_ARRAY_TASK_ID+0)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+1)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &
SPLIT=$((3*SLURM_ARRAY_TASK_ID+2)); root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE\) -b -q -l > ${LOGDIR}_${SLURM_ARRAY_TASK_ID}_reco.log  &

echo root ${VMCWORKDIR}\/macros\/run_reco_experiment.C\($RUN,$NTOTAL,$SPLIT,$NSPLIT,\"$GCData\",\"$GGData\",\{\},\"$MCFile\",\"$SupplePath\",30,\"$ParameterFile\",\"$VMCWORKDIR\/macros\/data\/${OUTPUT}\/\",kTRUE,${threshold}\) -b -q -l

#upload data into destination

wait
#echo scp -o ProxyCommand=\"ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu\" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+0)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION 
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+1)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION 
#SPLIT=$((3*SLURM_ARRAY_TASK_ID+2)); scp -o ProxyCommand="ssh -W %h:%p $FISHTANKUSER@nsclgw1.nscl.msu.edu" $VMCWORKDIR/macros/data/${OUTPUT}/*s$(printf "%02d" $SPLIT)* $FISHTANKUSER@fishtank:$DESTINATION 

cd ${VMCWORKDIR}/macros
if (( SLURM_ARRAY_TASK_ID == 0 )); then
    root ${VMCWORKDIR}\/macros\/embedMacros/run_general_reco.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -b -q -l > ${LOGDIR}_run.log 
fi


