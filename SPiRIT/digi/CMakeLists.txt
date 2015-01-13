# Create a library called "libSTDigi" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/SPiRIT/mc
${CMAKE_SOURCE_DIR}/SPiRIT/par
${CMAKE_SOURCE_DIR}/SPiRIT/format
${CMAKE_SOURCE_DIR}/SPiRIT/digi/PadResponse
${CMAKE_SOURCE_DIR}/SPiRIT/digi/DriftElectron
${CMAKE_SOURCE_DIR}/SPiRIT/digi
${CMAKE_SOURCE_DIR}/SPiRIT/reco/STConverter
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
DriftElectron/STDriftElectron.cc
DriftElectron/STWireResponse.cc
STDriftTask.cc
STProcessManager.cc
PadResponse/STPadResponse.cc
STPadResponseTask.cc
STElectronicsTask.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STDigiLinkDef.h)
Set(LIBRARY_NAME STDigi)
Set(DEPENDENCIES
  Base
  Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
  STMC
  STPar
  STFormat
  STReco
)

GENERATE_LIBRARY()
