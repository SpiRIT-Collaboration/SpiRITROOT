# Create a library called "libSTDummy" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${EIGEN3_INCLUDE_DIR}
${GENFIT2_INCLUDE_DIR}
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/ana
#${CMAKE_SOURCE_DIR}/embed
#${CMAKE_SOURCE_DIR}/mc
${CMAKE_SOURCE_DIR}/stack
${CMAKE_SOURCE_DIR}/format
${CMAKE_SOURCE_DIR}/par
${CMAKE_SOURCE_DIR}/generators
${CMAKE_SOURCE_DIR}/global
${CMAKE_SOURCE_DIR}/ana/MassCalculator
#${CMAKE_SOURCE_DIR}/tool
#${CMAKE_SOURCE_DIR}/reco
#${CMAKE_SOURCE_DIR}/reco/GETDecoder
#${CMAKE_SOURCE_DIR}/reco/STConverter
#${CMAKE_SOURCE_DIR}/reco/PulseShapeAnalyzer
#${CMAKE_SOURCE_DIR}/reco/HitClustering
#${CMAKE_SOURCE_DIR}/reco/SystemManipulator
#${CMAKE_SOURCE_DIR}/reco/RiemannTracking
#${CMAKE_SOURCE_DIR}/reco/HoughTracking
#${CMAKE_SOURCE_DIR}/reco/Genfit
#${CMAKE_SOURCE_DIR}/reco/Tracking
#${CMAKE_SOURCE_DIR}/reco/LinearTracking
#${CMAKE_SOURCE_DIR}/reco/CurveTracking
#${CMAKE_SOURCE_DIR}/reco/HelixTracking
#${CMAKE_SOURCE_DIR}/reco/Task
#${CMAKE_SOURCE_DIR}/reco/PID
${CMAKE_SOURCE_DIR}/reco/BDCMagneticProjection
)

include_directories(${INCLUDE_DIRECTORIES})

include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${GENFIT2_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
STAnaParticleDB.cc
ST_TrackCut.cc
ST_ClusterNum_DB.cc
ST_ProduceDB_ClusterNum.cc
STPIDAnalysisTask.cc
STConcReaderTask.cc
STPIDCutTask.cc
STPIDProbTask.cc
STPIDMachineLearningTask.cc
STTransformFrameTask.cc
STEfficiencyTask.cc
STPhiEfficiencyTask.cc
EfficiencyFactory.cc
STMatchImQMDTask.cc
STFilterTask.cc
STFilterEventTask.cc
STAddBDCInfoTask.cc
STSimpleGraphsTask.cc
MassCalculator/STMassCalSimpleBB.cc
MassCalculator/STBBFunction.cc
STAnalysisFactory.cc
STERATTask.cc
STModelReaderTask.cc
STReactionPlaneTask.cc
STAzimuthalCorRPTask.cc
STDivideEventTask.cc
STPiProbTask.cc
STCSVReaderTask.cc
STObsWriterTask.cc
STUrQMDFormatWriterTask.cc
STImpactParameterMLTask.cc
ProcessPipe.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STAnaLinkDef.h)
Set(LIBRARY_NAME STAna)
Set(DEPENDENCIES
    Base
    Stack
    XMLParser
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
   STReco
   STFormat
   STPar
   STGlobal
   genfit2
   STGen
)

GENERATE_LIBRARY()
