# Create a library called "libSPiRIT" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
#put here all directories where header files are located
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/SPiRIT/mc
${CMAKE_SOURCE_DIR}/SPiRIT/stack
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
STContFact.cc
STGeoPar.cc
STDigiPar.cc
STGas.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STParLinkDef.h)
Set(LIBRARY_NAME STPar)
Set(DEPENDENCIES
    Base
    Stack
)

GENERATE_LIBRARY()
