#!/usr/bin/env bash
#SBATCH -o "./log/slurm_%A.log"
source ${VMCWORKDIR}/build/config.sh

INPUT=$1
VERTEX=$2
MOMENTUM=$3
PARTICLE=$4
PHI=$5
THETA=$6
NEXTJOB=$7
CONFIGLIST=$8

# Create input config file
FILENAME=.temp_${NEXTJOB}
SUBMITFILE=${VMCWORKDIR}/parameters/${FILENAME}

echo ${SUBMITFILE}

cat > $SUBMITFILE << EOF
Momentum        0.0     0.0     ${MOMENTUM} # in GeV/Z
VertexFile      ${VERTEX}
Particle        ${PARTICLE} # 211 for pion for some reasons
Theta           ${THETA}
Phi             ${PHI}
EOF

# Creat directory for logging 
LOGDIR=log\/${INPUT}
mkdir -p "${LOGDIR%/*}"

cd ${VMCWORKDIR}/macros
root run_mc.C\(\"$INPUT\",-1,\"\",\"data/\",kTRUE,\"$FILENAME\"\) -b -q -l > ${LOGDIR}_mc.log
rm -f $SUBMITFILE

root run_digi.C\(\"$INPUT\"\) -b -q -l > ${LOGDIR}_digi.log
root run_general.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > ${LOGDIR}_run.log 
