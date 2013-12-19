# Create a library called "libSPiRIT" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
#put here all directories where header files are located
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/base
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/SPiRIT/tpc
${CMAKE_SOURCE_DIR}/SPiRIT/stack
)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
SPiRIT.cxx
SPiRITContFact.cxx
SPiRITGeo.cxx
SPiRITGeoPar.cxx
SPiRITPoint.cxx
)

Set(LINKDEF SPiRITLinkDef.h)
Set(LIBRARY_NAME SPiRIT)
Set(DEPENDENCIES
    Base
    Stack
)

GENERATE_LIBRARY()
