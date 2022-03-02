cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( BasicWarping )
set( APP_NAME "${PROJECT_NAME}App" )

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../.." ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

ci_make_app(
	APP_NAME ${APP_NAME}
	SOURCES ${APP_PATH}/src/_TBOX_PREFIX_App.cpp
	CINDER_PATH ${CINDER_PATH}
	BLOCKS Cinder-Warping
)

get_target_property( OUTPUT_DIR ${APP_NAME} RUNTIME_OUTPUT_DIRECTORY )

if ( APPLE )
	add_custom_target( run
		COMMAND open ${OUTPUT_DIR}/${APP_NAME}.app
		DEPENDS ${OUTPUT_DIR}/${APP_NAME}.app/Contents/MacOS/${APP_NAME}
		WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
	)
elseif( UNIX )
	add_custom_target( run
		COMMAND ${OUTPUT_DIR}/${APP_NAME}
		DEPENDS ${OUTPUT_DIR}/${APP_NAME}
		WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
	)
endif()
