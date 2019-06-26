# Create a library called "libSTMC" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/r3b_neuland
${CMAKE_SOURCE_DIR}/r3b_neuland/r3bbase
${CMAKE_SOURCE_DIR}/r3b_neuland/r3bdata
${CMAKE_SOURCE_DIR}/r3b_neuland/neuland
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
    R3BNeuland.cxx
    R3BNeulandMCMon.cxx
    ElasticScattering.cxx
    IsElastic.cxx
    R3BNeulandNeutron2DPar.cxx
    R3BNeulandGeoPar.cxx
    R3BNeulandContFact.cxx
    R3BNeulandVisualizer.cxx
)

#CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")
change_file_extension(*.cxx *.h HEADERS "${SRCS}")

set(HEADERS
    ClusteringEngine.h
    #ElasticScattering.h
    Filterable.h
    TCAConnector.h
    Validated.h
    #IsElastic.h
    R3BNeulandNeutron2DPar.h
    R3BNeulandGeoPar.h
    R3BNeulandContFact.h
    R3BNeulandVisualizer.h)


Set(LINKDEF NeulandSimulationLinkDef.h)
Set(LIBRARY_NAME NLSIM)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
    R3BBASE
    R3BDATA
)

GENERATE_LIBRARY()
