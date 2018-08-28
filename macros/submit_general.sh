#!/usr/bin/env bash
#SBATCH -o "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/log/slurm_%A.log"
source /mnt/spirit/analysis/user/tsangc/SpiRITROOT/build/config.sh

INPUT=$1
VERTEX=$2
MOMENTUM=$3
PARTICLE=$4
PHI=$5
THETA=$6
NEXTJOB=$7
CONFIGLIST=$8

# Create input config file
FILENAME=temp_${NEXTJOB}
SUBMITFILE=${VMCWORKDIR}/parameters/${FILENAME}

echo ${SUBMITFILE}

cat > $SUBMITFILE << EOF
NEvent          ${NEVENT}
Momentum        0.0     0.0     ${MOMENTUM} # in GeV/Z
VertexFile      ${VERTEX}
Particle        ${PARTICLE} # 211 for pion for some reasons
Theta           ${THETA}
Phi             ${PHI}
EOF

cd /mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros
root run_mc.C\(\"$INPUT\",-1,\"\",\"data/\",kTRUE,\"$FILENAME\"\) -b -q -l > log\/${INPUT}_mc.log
rm -f $SUBMITFILE

root run_digi.C\(\"$INPUT\"\) -b -q -l > log\/${INPUT}_digi.log
root run_general.C\(\"$CONFIGLIST\"\,$NEXTJOB,1\) -q -l > log\/${INPUT}_run.log 
