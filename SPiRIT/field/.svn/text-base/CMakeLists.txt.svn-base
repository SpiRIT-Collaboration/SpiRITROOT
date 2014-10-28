# Create a library called "libBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${CMAKE_SOURCE_DIR}/fairtools
${CMAKE_SOURCE_DIR}/base 
${CMAKE_SOURCE_DIR}/field 
${CMAKE_SOURCE_DIR}/parbase 
)

include_directories( ${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(FIELD_SRCS
PndConstField.cxx       
PndFieldContFact.cxx 
PndFieldMap.cxx      
PndFieldMapData.cxx  
PndSolenoidMap.cxx   
PndFieldPar.cxx      
PndFieldCreator.cxx  
PndDipoleMap.cxx     
PndTransMap.cxx      
PndRegion.cxx        
PndDipolePar.cxx     
PndDipole1Par.cxx     
PndDipole2Par.cxx     
PndDipole3Par.cxx     
PndTransPar.cxx
PndSolenoidPar.cxx
PndSolenoid1Par.cxx
PndSolenoid2Par.cxx
PndSolenoid3Par.cxx   
PndSolenoid4Par.cxx
PndMapPar.cxx        
PndMultiFieldPar.cxx 
PndConstPar.cxx    
PndMultiField.cxx  
)

if(RULE_CHECKER_FOUND)
  CHECK_RULES("${FIELD_SRCS}" "${INCLUDE_DIRECTORIES}" FIELD_RULES)
endif(RULE_CHECKER_FOUND)

# fill list of header files from list of source files
# by exchanging the file extension
CHANGE_FILE_EXTENSION(*.cxx *.h FIELD_HEADERS "${FIELD_SRCS}")

set(FIELD_LINKDEF FieldLinkDef.h)
set(FIELD_DICTIONARY ${CMAKE_CURRENT_BINARY_DIR}/FieldDict.cxx) 

ROOT_GENERATE_DICTIONARY("${FIELD_HEADERS}" "${FIELD_LINKDEF}" "${FIELD_DICTIONARY}" "${INCLUDE_DIRECTORIES}")


set(FIELD_SRCS ${FIELD_SRCS} ${FIELD_DICTIONARY})

add_library(Field SHARED ${FIELD_SRCS})
target_link_libraries(Field ${ROOT_LIBRARIES})
set_target_properties(Field PROPERTIES ${FAIRROOT_LIBRARY_PROPERTIES})
################ install ###################
install(TARGETS Field DESTINATION ${CMAKE_BINARY_DIR}/lib)
