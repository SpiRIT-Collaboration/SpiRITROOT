# Create a library called "libSTEventDisplay" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
${CMAKE_SOURCE_DIR}/eventdisplay
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/SPiRIT/eventdisplay
${CMAKE_SOURCE_DIR}/SPiRIT/format
${CMAKE_SOURCE_DIR}/SPiRIT/reco/RiemannTracking
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
STEventManager.cc
STEventManagerEditor.cc
STEventDrawTask.cc
STEventDraw.cc
STHitDraw.cc
STHitClusterDraw.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STEventDisplayLinkDef.h) # Change the linkdef name and delete this comment
Set(LIBRARY_NAME STEventDisplay) # Change the library name and delete this comment
Set(DEPENDENCIES
    Base
    Stack
    EventDisplay
# ROOT Eve package
    Eve
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
    STFormat
    STReco
)

GENERATE_LIBRARY()
