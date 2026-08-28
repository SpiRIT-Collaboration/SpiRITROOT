#source /home/jesus/Desktop/SpROOT/act_env/curtis/SpiRITROOT/build/config.sh
#cd ~/Desktop/SpROOT/sim_2025/
#echo "DEBUG: All parameters: $*"
#echo "DEBUG: Parameter count: $#"
#echo "DEBUG: \$1=$1, \$2=$2, \$3=$3, \$4=$4, \$5=$5, \$6=$6, \$7=$7"
nEvents=$1
particle=$2
mom=$3
magnetic=$4
simulateBeam=$5
layercut=$6
gainFile=$7
#echo "DEBUG: gainFile after assignment = '${gainFile}'"

# Set default layer cut if not provided
if [ -z "${layercut}" ]; then
    layercut=112
fi

# Set default gain file if not provided
if [ -z "${gainFile}" ]; then
    gainFile=0
fi

# Convert simulateBeam from numeric (1/0) to boolean (true/false)
if [ "${simulateBeam}" = "1" ]; then
    simulateBeam="true"
elif [ "${simulateBeam}" = "0" ]; then
    simulateBeam="false"
fi

# Set gain file based on input parameter
if [ "${gainFile}" = "1" ]; then
    gainFileName="RelativeGain.list"
else
    gainFileName="RelativeGain_Same.list"
fi

# Convert particle name to numeric code for ROOT macro
case "${particle}" in
    "proton")
        particleCode=1
        ;;
    "2"|"deutron"|"deuteron")
        particleCode=2
        ;;
    "3"|"3H"|"triton")
        particleCode=3
        ;;
    "4"|"4He"|"alpha")
        particleCode=4
        ;;
    "pion_plus"|"pi+")
        particleCode=5
        ;;
    "pion_minus"|"pi-")
        particleCode=6
        ;;
    "6Li")
        particleCode=7
        ;;
    "7Li")
        particleCode=8
        ;;
    *)
        echo "Error: Unknown particle type '${particle}'"
        echo "Supported: proton, 2/deutron/deuteron, 3/3H/triton, 4/4He/alpha, pion_plus/pi+, pion_minus/pi-, 6Li, 7Li"
        exit 1
        ;;
esac

if [ -z ${nEvents} ]; then
    echo "./run_sims.sh nEvents particle mom magnetic simulateBeam(1=true,0=false) [layercut=112] [gainFile=0]"
    echo "Particles: proton, 2/deutron/deuteron, 3/3H/triton, 4/4He/alpha, pion_plus/pi+, pion_minus/pi-, 6Li, 7Li"
    echo "GainFile: 0=RelativeGain_Same.list, 1=RelativeGain.list"
    echo "Example: ./run_sims.sh 200 3H 2.3 1 0 112 0"
    exit
fi


echo "particle = ${particle} (code: ${particleCode})"
echo "momentum = ${mom}"
echo "magnetic = ${magnetic}"
echo "simulateBeam = ${simulateBeam}"
echo "layercut = ${layercut}"
echo "gainFile = ${gainFile} (${gainFileName})"

if [ ${magnetic} -gt 0 ]; then
    MFmap=kTRUE
    echo "Using field map "${magnetic}
else
    MFmap=kFALSE
    echo "Using constant magnetic field"
fi
filename="sims_${nEvents}_MFmap_${MFmap}_MF_${magnetic}_Part_${particle}_Mom_${mom}_SimBeam_${simulateBeam}_zcut_${layercut}_gain_${gainFile}"
#filename="sims_${nEvents}_MFmap${MFmap}_MF${magnetic}_SimBeam${simulatebeam}_Part${particle}_Mom${mom}"
echo "Generated filename: ${filename}"
#echo "DATE "`date` >> ${filename}.log
#root -b -q "./run_mc_simulations.C(\"${filename}\",${nEvents},${mom},${particleCode},${magnetic},\"\",\"./data_sim/\",kTRUE)" &> "./data_sim/mc_fulllogs_${filename}.log"
echo "DATE "`date` >> ${filename}.log
root -b -q "./run_digi.C(\"${filename}\",1234,${simulateBeam},${magnetic},-1,\"ST.parameters.par\",\"${gainFileName}\")" &> "./data_sim/digi_fulllogs_${filename}.log"
echo "DATE "`date` >> ${filename}.log
#root -b -q "./run_reco_all_mc.C(1234,${layercut},\"${filename}\",${magnetic},${nEvents},${nEvents},0,${simulateBeam},\"\",30,\"ST.parameters.fullmc.par\",\"\",\"./data_sim/\",0,\"${gainFileName}\")" &> "./data_sim/reco_fulllogs_${filename}.log"
root -b -q "./run_reco_sim2024.C(1234,${layercut},\"${filename}\",${magnetic},${nEvents},${nEvents},0,${simulateBeam},\"\",30,\"ST.parameters.fullmc.par\",\"\",\"./data_sim/\",0,\"${gainFileName}\")" &> "./data_sim/reco_fulllogs_${filename}.log"
#echo "DATE "`date` >> ${filename}.log
#echo "Expected analysis input file: ./data/Simulations/${filename}_s0.layer_cut.${layercut}.reco.v1.04.root"
#echo "Expected analysis output file: ./sim_results/analysis_${filename}_s0.layer_cut.${layercut}.reco.v1.04.root"
#if [ -f "./data/Simulations/${filename}_s0.layer_cut.${layercut}.reco.v1.04.root" ]; then
#    echo "Analysis input file exists, proceeding with analysis"
#    root -b -q "./Analysis_sim.C(\"./data/Simulations/${filename}_s0.layer_cut.${layercut}.reco.v1.04.root\",\"./sim_results/analysis_${filename}_s0.layer_cut.${layercut}.reco.v1.04.root\",${layercut})" &> "./data/Simulations/anal_fulllogs_${filename}.log"
#else
#    echo "ERROR: Analysis input file does not exist: ./data/Simulations/${filename}_s0.layer_cut.${layercut}.reco.v1.04.root"
#fi
 
