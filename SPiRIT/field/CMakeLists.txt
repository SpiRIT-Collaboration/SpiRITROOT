# Create a library called "libSTField" which includes the source files given in the array.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${BASE_INCLUDE_DIRECTORIES}
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/base
${CMAKE_SOURCE_DIR}/parbase
${CMAKE_SOURCE_DIR}/geobase
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/SPiRIT/mc
${CMAKE_SOURCE_DIR}/SPiRIT/field
${CMAKE_SOURCE_DIR}/SPiRIT/stack
${CMAKE_SOURCE_DIR}/SPiRIT/format
${CMAKE_SOURCE_DIR}/SPiRIT/par
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
STFieldMap.cc      
STFieldMapData.cc  
STFieldPar.cc
STConstField.cc
STConstPar.cc      
STMapPar.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STFieldLinkDef.h)
Set(LIBRARY_NAME STField)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
   STMC
   STDigi
   STPar
   STFormat
)

GENERATE_LIBRARY()
