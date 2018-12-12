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
#${CMAKE_SOURCE_DIR}/par
${CMAKE_SOURCE_DIR}/global
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
#${CMAKE_SOURCE_DIR}/reco/BDCMagneticProjection
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
ST_TrackCut.cc
ST_ClusterNum_DB.cc
HistToCutG.cc
Rules.cc
RulesDrawer.cc
RecoRules.cc
RuleTree.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STAnaLinkDef.h)
Set(LIBRARY_NAME STAna)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
   STFormat
#   STPar
   STGlobal
   genfit2
)

GENERATE_LIBRARY()
