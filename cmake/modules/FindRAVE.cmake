# - Find RAVE instalation
# This module tries to find the RAVE installation on your system.
#
# Variables defined by this module:
#
#   RAVE_FOUND               System has RAVE
#   RAVE_INCLUDE_DIR         RAVE include directories: not cached
#   RAVE_LIBRARY_DIR         The path to where the RAVE library files are.
#

Message(STATUS "Looking for RAVE...")

Set(RAVE_VERSION_SEARCHPATH
  ${SIMPATH}/bin
)

Set(RAVE_FOUND FALSE)
Set(RAVE_DEFINITIONS "")
Set(RAVE_INSTALLED_VERSION_TOO_OLD FALSE)

Set(RAVE_FIND_VERSION_MAJOR 0)
Set(RAVE_FIND_VERSION_MINOR 6)
Set(RAVE_FIND_VERSION_PATCH 21)

Find_Program(RAVE_VERSION_EXECUTABLE NAMES rave_version
             PATHS ${RAVE_VERSION_SEARCHPATH}
             NO_DEFAULT_PATH
            )
     
If(RAVE_VERSION_EXECUTABLE)
   
  String(REGEX REPLACE "(^.*)/bin/rave_version" "\\1" test ${RAVE_VERSION_EXECUTABLE}) 

  Execute_Process(COMMAND ${RAVE_VERSION_EXECUTABLE}
                  OUTPUT_VARIABLE RAVE_VERSION_STRING
                 )

  String(STRIP ${RAVE_VERSION_STRING} RAVE_VERSION_STRING)
  String(REGEX MATCH "[0-9].[0-9].[0-9][0-9]" RAVE_VERSION_STRING "${RAVE_VERSION_STRING}")

  MESSAGE(STATUS "Looking for RAVE... - found ${SIMPATH}/lib")
  MESSAGE(STATUS "Looking for RAVE... - found version is ${RAVE_VERSION_STRING} ")   
   
  # extract major, minor, and patch versions from
  # the version string given by rave_version
  String(REGEX REPLACE "^([0-9])+\\.[0-9]+\\.[0-9][0-9]+$" "\\1" RAVE_VERSION_MAJOR "${RAVE_VERSION_STRING}")
  String(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9][0-9]+$" "\\1" RAVE_VERSION_MINOR "${RAVE_VERSION_STRING}")
  String(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9][0-9]+)$" "\\1" RAVE_VERSION_PATCH "${RAVE_VERSION_STRING}")

  # compute overall version numbers which can be compared at once
  Math(EXPR req_vers "${RAVE_FIND_VERSION_MAJOR}*10000 + ${RAVE_FIND_VERSION_MINOR}*100 + ${RAVE_FIND_VERSION_PATCH}")
  Math(EXPR found_vers "${RAVE_VERSION_MAJOR}*10000 + ${RAVE_VERSION_MINOR}*100 + ${RAVE_VERSION_PATCH}")

  Set(RAVE_Version ${found_vers})
  Set(RAVE_VERSION_NUMBER ${found_vers})

  If(found_vers LESS req_vers)
    Set(RAVE_FOUND FALSE)
    Set(RAVE_INSTALLED_VERSION_TOO_OLD TRUE)
  Else(found_vers LESS req_vers)
    Set(RAVE_FOUND TRUE)
  EndIf(found_vers LESS req_vers)

Else(RAVE_VERSION_EXECUTABLE)
  Message(STATUS "Looking for RAVE... - Not found")
  Message(FATAL_ERROR "RAVE not installed in the searchpath and RAVESYS is not set. Please set RAVESYS or add the path to your RAVE installation in the Macro FindRAVE.cmake in the subdirectory cmake/modules.")
Endif(RAVE_VERSION_EXECUTABLE)

If(RAVE_FOUND)

  # Set RAVE_LIBRARY_DIR
  Set(RAVE_LIBRARY_DIR ${SIMPATH}/lib)
  Set(RAVE_LDFLAGS "-L${SIMPATH}/lib -lRaveBase -lRaveCore -lRaveVertex -lRaveFlavorTag -lRaveVertexKinematics")

  # Set RAVE_INCLUDES
  Set(RAVE_INCLUDE_DIR ${SIMPATH}/include/rave)

  # Make variables changeble to the advanced user
  Mark_As_Advanced(RAVE_LIBRARY_DIR RAVE_INCLUDE_DIR)

  Set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${RAVE_LIBRARY_DIR})

Else(RAVE_FOUND)

  If(RAVE_FIND_REQUIRED)
    Message(STATUS "Looking for RAVE... - found version to old.")
    Message(STATUS "Looking for RAVE... - Minimum required version is ${RAVE_FIND_VERSION}")
    Message(FATAL_ERROR "Stop here because of a wrong Root version.")
  EndIf(RAVE_FIND_REQUIRED)

Endif(RAVE_FOUND)
