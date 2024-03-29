# Create a library called "libSTEventDisplay" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
${EIGEN3_INCLUDE_DIR}
${GENFIT2_INCLUDE_DIR}
${ANAROOT_INCLUDE_DIR}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/eventdisplay
${CMAKE_SOURCE_DIR}/format
${CMAKE_SOURCE_DIR}/par
${CMAKE_SOURCE_DIR}/reco
${CMAKE_SOURCE_DIR}/tool
${CMAKE_SOURCE_DIR}/util
${CMAKE_SOURCE_DIR}/reco/PulseShapeAnalyzer
${CMAKE_SOURCE_DIR}/reco/STConverter
${CMAKE_SOURCE_DIR}/reco/GETDecoder
${CMAKE_SOURCE_DIR}/reco/RiemannTracking
${CMAKE_SOURCE_DIR}/reco/LinearTracking
${CMAKE_SOURCE_DIR}/reco/CurveTracking
${CMAKE_SOURCE_DIR}/reco/Genfit
${CMAKE_SOURCE_DIR}/global
${CMAKE_SOURCE_DIR}/field
)

include_directories(${INCLUDE_DIRECTORIES})

include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${GENFIT2_LIBRARY_DIR}
${ANAROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
STEventManager.cc
STEventManagerEditor.cc
STEventDrawTask.cc
STEveManager.cc
STEveTask.cc
STEveDrawTask.cc
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
    STPar
    STUtil
    anaroot
)

GENERATE_LIBRARY()
