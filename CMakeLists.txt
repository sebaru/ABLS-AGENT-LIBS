cmake_minimum_required(VERSION 3.16)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ABLS_SATELLITE_LIBS_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT ABLS_SATELLITE_LIBS_VERSION)
  set(ABLS_SATELLITE_LIBS_VERSION "unknown")
endif()

if(ABLS_SATELLITE_LIBS_VERSION MATCHES "^v?([0-9]+\\.[0-9]+)")
  set(ABLS_SATELLITE_LIBS_VERSION_NUMBER "${CMAKE_MATCH_1}")
else()
  set(ABLS_SATELLITE_LIBS_VERSION_NUMBER "1.0")
endif()

project(abls-satellite-libs
  VERSION ${ABLS_SATELLITE_LIBS_VERSION_NUMBER}
  DESCRIPTION "Abls Habitat reusable satellite lifecycle library"
  HOMEPAGE_URL "https://docs.abls-habitat.fr"
  LANGUAGES C
)

include(GNUInstallDirs)
include(CheckIncludeFile)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(ABLS_LIBS REQUIRED abls-libs)

find_library(ABLS_LIBS_EXACT
  NAMES abls-libs
  PATHS /usr/local/lib64 /usr/local/lib
  NO_DEFAULT_PATH
)

set(CMAKE_REQUIRED_INCLUDES ${ABLS_LIBS_INCLUDE_DIRS})
check_include_file(abls-libs.h)

add_library(abls-satellite-libs SHARED
  src/satellite.c
)

set_target_properties(abls-satellite-libs PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "include/abls_satellite_libs.h"
)

target_include_directories(abls-satellite-libs
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/abls-satellite-libs>
  PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${GTHREAD_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${ABLS_LIBS_INCLUDE_DIRS}
)

target_compile_options(abls-satellite-libs PRIVATE
  -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -pipe -g -ggdb
)

target_link_libraries(abls-satellite-libs PRIVATE
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${ABLS_LIBS_EXACT}
)

install(TARGETS abls-satellite-libs
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/abls-satellite-libs COMPONENT devel
)

configure_file(abls-satellite-libs.pc.in abls-satellite-libs.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/abls-satellite-libs.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT devel
)


set(CPACK_PACKAGE_NAME "abls-satellite-libs")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_VENDOR "Abls-Habitat")
set(CPACK_PACKAGE_CONTACT "sebastien.lefevre@abls-habitat.fr")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_LICENSE "GPLv3")

set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime devel)

set(CPACK_RPM_RUNTIME_PACKAGE_NAME "abls-satellite-libs")
set(CPACK_RPM_RUNTIME_PACKAGE_SUMMARY "Abls-Habitat satellite lifecycle library (runtime)")
set(CPACK_RPM_RUNTIME_PACKAGE_REQUIRES "glib2, json-glib, abls-libs")
set(CPACK_RPM_RUNTIME_POST_SCRIPTLET "/sbin/ldconfig")
set(CPACK_RPM_RUNTIME_FILE_NAME "abls-satellite-libs-${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}.rpm")

set(CPACK_RPM_DEVEL_PACKAGE_NAME "abls-satellite-libs-devel")
set(CPACK_RPM_DEVEL_PACKAGE_SUMMARY "Abls-Habitat satellite lifecycle library (development files)")
set(CPACK_RPM_DEVEL_PACKAGE_REQUIRES "abls-satellite-libs = ${PROJECT_VERSION}, glib2-devel, json-glib-devel, abls-libs-devel")
set(CPACK_RPM_DEVEL_FILE_NAME "abls-satellite-libs-devel-${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}.rpm")

include(CPack)
