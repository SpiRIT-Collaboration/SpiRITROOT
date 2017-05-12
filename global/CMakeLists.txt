set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR}
${GENFIT2_INCLUDE_DIR}
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/global
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
STPID.cc
#STConfigRun.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STGlobalLinkDef.h)
Set(LIBRARY_NAME STGlobal)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory
)

GENERATE_LIBRARY()
