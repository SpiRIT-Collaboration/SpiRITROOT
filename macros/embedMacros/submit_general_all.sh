cd ${VMCWORKDIR}\/macros

RECOFILE=$1
OUTPUT=$2
NEXTJOB=$3
CONFIGLIST=$4
MCFILE=$5
DESTINATION=$6
FISHTANKUSER=$7
CONFIGFILE=$8
MCSUBMITFILE=$9
CLEANUPFILE=${10}
NTOTAL=${11}

# Creat directory for logging 
MCID=$(sbatch --parsable ${MCSUBMITFILE} ${OUTPUT} ${CONFIGLIST} ${CONFIGFILE} ${NTOTAL})
MERGEID=$(sbatch --parsable --dependency=afterok:${MCID} --kill-on-invalid-dep=yes embedMacros/submit_merge.sh ${OUTPUT})
#sbatch --job-name="CleanupFailSafe" --dependency=afternotok:${MERGEID} --kill-on-invalid-dep=yes ${CLEANUPFILE} ${OUTPUT} ${CONFIGLIST} ${NEXTJOB} ${FISHTANKUSER} ${DESTINATION}
RECOID=$(sbatch --parsable --dependency=afterok:${MERGEID} --kill-on-invalid-dep=yes ${RECOFILE} ${OUTPUT} ${NEXTJOB} ${CONFIGLIST} ${MCFILE} ${DESTINATION} ${FISHTANKUSER} ${NTOTAL})
sbatch --job-name="Cleanup" --dependency=afterany:${RECOID} --kill-on-invalid-dep=yes ${CLEANUPFILE} ${OUTPUT} ${CONFIGLIST} ${NEXTJOB} ${FISHTANKUSER} ${DESTINATION}
#sbatch ${RECOFILE} ${OUTPUT} ${NEXTJOB} ${CONFIGLIST} ${MCFILE} ${DESTINATION} ${FISHTANKUSER} ${NTOTAL}
