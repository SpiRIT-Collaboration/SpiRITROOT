# Create a library called "libSTFormat" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
#put here all directories where header files are located
${CMAKE_SOURCE_DIR}/SPiRIT/reco/GETDecoder
)

include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
GETDecoder.cc
GETFrame.cc
GETMath.cc
GETPlot.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF GETDecoderLinkDef.h)
Set(LIBRARY_NAME GETDecoder)

GENERATE_LIBRARY()
