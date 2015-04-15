IF(DEFINED SSC_ROOT)
    IF(NOT ${SSC_ROOT} STREQUAL "")
        MESSAGE(STATUS "Using manually specified SSC_ROOT : ${SSC_ROOT}")
        FIND_PACKAGE(ssc REQUIRED COMPONENTS random_data_generator numeric NO_CMAKE_ENVIRONMENT_PATH HINTS "${SSC_ROOT}")
    ELSE()
        FIND_PACKAGE(ssc REQUIRED COMPONENTS random_data_generator numeric)
    ENDIF()
ELSE()
    FIND_PACKAGE(ssc REQUIRED COMPONENTS random_data_generator numeric)
ENDIF()

IF(ssc_FOUND)
    MESSAGE(STATUS "SSC : ssc_INCLUDE_DIRS = ${ssc_INCLUDE_DIRS}")
    MESSAGE(STATUS "SSC : ssc_VERSION = ${ssc_VERSION}")
ENDIF()

MACRO(ADD_IMPORTED_LIB libname)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${libname})
    GET_TARGET_PROPERTY(MYLIB_LOCATION ${libname} LOCATION)
    INSTALL(FILES ${MYLIB_LOCATION} DESTINATION ${LIBRARY_OUTPUT_DIRECTORY})
ENDMACRO()
