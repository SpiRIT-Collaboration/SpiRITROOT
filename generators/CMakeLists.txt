# Create a library called "libSTGen" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/generators
${CMAKE_SOURCE_DIR}/format

)

include_directories(${INCLUDE_DIRECTORIES})

include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
STEventGenGenerator.cc
STSimpleEventGenerator.cc
STSingleTrackGenerator.cc
STPHITSEventGenerator.cc
STTransportModelEventGenerator.cc
STModelToLabFrameGenerator.cc
UrQMDParticle.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STGenLinkDef.h)
Set(LIBRARY_NAME STGen)
Set(DEPENDENCIES
  Base
  Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
  Gen
)

GENERATE_LIBRARY()
