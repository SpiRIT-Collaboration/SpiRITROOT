# Create a library called "libSTMC" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
#${CMAKE_SOURCE_DIR}/r3b_neuland
${CMAKE_SOURCE_DIR}/r3b_neuland/r3bbase
${CMAKE_SOURCE_DIR}/r3b_neuland/r3bdata
#${CMAKE_SOURCE_DIR}/r3b_neuland/r3bdata/neulandData
${CMAKE_SOURCE_DIR}/stack
${CMAKE_SOURCE_DIR}/format
${CMAKE_SOURCE_DIR}/par
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
    R3BMCStack.cxx
    R3BMCTrack.cxx
    R3BHit.cxx

    R3BPaddleTamexMappedData.cxx
    R3BNeulandPoint.cxx
    R3BNeulandHit.cxx
    R3BNeulandCluster.cxx
    R3BNeulandNeutron.cxx
    R3BNeulandMappedData.cxx
    R3BNeulandCalData.cxx

    R3BCalifaCrystalCalData.cxx
    R3BCalifaCrystalCalDataSim.cxx
    R3BCalifaMappedData.cxx
    R3BCalifaHitData.cxx
    R3BCalifaHitDataSim.cxx
    R3BCalifaPoint.cxx
   )

change_file_extension(*.cxx *.h HEADERS "${SRCS}")
set(HEADERS ${HEADERS} R3BDetectorList.h)

Set(LINKDEF R3BDataLinkDef.h)
Set(LIBRARY_NAME R3BDATA)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
    R3BBASE
)

GENERATE_LIBRARY()
