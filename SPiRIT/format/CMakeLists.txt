# Create a library called "libSTFormat" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
#put here all directories where header files are located
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/SPiRIT/format
)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
STMCPoint.cc
STRawEvent.cc
STPad.cc
STEvent.cc
STHit.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STFormatLinkDef.h)
Set(LIBRARY_NAME STFormat)
Set(DEPENDENCIES
    Base
    Stack
)

GENERATE_LIBRARY()
