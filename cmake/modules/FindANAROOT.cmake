# - Find ANAROOT instalation
# This module tries to find the ANAROOT installation on your system.
#
# Variables defined by this module:
#
#   ANAROOT_FOUND               System has ANAROOT
#   ANAROOT_INCLUDE_DIR         ANAROOT include directories: not cached
#   ANAROOT_LIBRARY_DIR         The path to where the ANAROOT library files are.
#

Message(STATUS "Looking for ANAROOT...")

Set(ANAROOT_LIBRARY_SEARCHPATH
  ${SIMPATH}/lib
)

Set(ANAROOT_FOUND FALSE)

Find_Library(ANAROOT_LIBRARY NAMES anaroot
             PATHS ${ANAROOT_LIBRARY_SEARCHPATH}
             NO_DEFAULT_PATH
            )

If(ANAROOT_LIBRARY)

  MESSAGE(STATUS "Looking for ANAROOT... - found ${SIMPATH}/lib")

  Set(ANAROOT_LIBRARY_DIR ${SIMPATH}/lib)
  Set(ANAROOT_LDFLAGS "-L${SIMPATH}/lib -lanaroot")

  Set(ANAROOT_INCLUDE_DIR ${SIMPATH}/include)

  Mark_As_Advanced(ANAROOT_LIBRARY_DIR ANAROOT_INCLUDE_DIR)

  Set(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${ANAROOT_LIBRARY_DIR})

  Set(ANAROOT_FOUND TRUE)

Else(ANAROOT_LIBRARY)

  If(ANAROOT_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Looking for ANAROOT... - Not found!")
  EndIf(ANAROOT_FIND_REQUIRED)

EndIf(ANAROOT_LIBRARY)
