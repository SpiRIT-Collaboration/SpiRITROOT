# Create a library called "libBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${BASE_INCLUDE_DIRECTORIES}
${CMAKE_SOURCE_DIR}/SPiRIT/stack
)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${CMAKE_SOURCE_DIR}/base
${CMAKE_SOURCE_DIR}/SPiRIT/stack

)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
STStack.cxx 
STMCTrack.cxx
)

Set(HEADERS )
Set(LINKDEF MCStackLinkDef.h)
Set(LIBRARY_NAME Stack)
Set(DEPENDENCIES Base EG Physics Cint Core)

GENERATE_LIBRARY()

