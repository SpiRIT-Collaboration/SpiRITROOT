# Create a library called "libSTReco" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/SPiRIT/mc
${CMAKE_SOURCE_DIR}/SPiRIT/stack
${CMAKE_SOURCE_DIR}/SPiRIT/par
${CMAKE_SOURCE_DIR}/SPiRIT/format
${CMAKE_SOURCE_DIR}/SPiRIT/reco
${CMAKE_SOURCE_DIR}/SPiRIT/reco/GETDecoder
${CMAKE_SOURCE_DIR}/SPiRIT/reco/STConverter
${CMAKE_SOURCE_DIR}/SPiRIT/reco/PulseShapeAnalyzer
${CMAKE_SOURCE_DIR}/SPiRIT/reco/RiemannTracking
${CMAKE_SOURCE_DIR}/SPiRIT/reco/SystemManipulator
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
GETDecoder/GETDecoder.cc
GETDecoder/GETFrame.cc
GETDecoder/GETMath.cc
GETDecoder/GETPlot.cc

STConverter/STCore.cc
STConverter/STPedestal.cc
STConverter/STMap.cc

PulseShapeAnalyzer/STPSASimple.cc

#HitClusterizer/STHitClusteringSimple.cc

SystemManipulator/STSystemManipulator.cc

#RiemannTracking/STRiemannHit.cc
#RiemannTracking/STRiemannTrack.cc

STDecoderTask.cc
STPSATask.cc
STHitClusteringTask.cc
#STRiemannTrackingTask.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STRecoLinkDef.h)
Set(LIBRARY_NAME STReco)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
    STFormat
    STPar
)

GENERATE_LIBRARY()
