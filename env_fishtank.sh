export SIMPATH="/mnt/spirit/analysis/user/tsangc/NewSpiRITROOT"
export PATH=$PATH:$SIMPATH/bin
export MANPATH=$MANPATH:$SIMPATH/share
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SIMPATH/lib
source $SIMPATH/share/Geant4-10.2.1/geant4make/geant4make.sh
source $SIMPATH/bin/thisroot.sh
export FAIRROOTPATH="/mnt/spirit/analysis/user/tsangc/NewSpiRITROOT/FairRoot"
source build/config.sh