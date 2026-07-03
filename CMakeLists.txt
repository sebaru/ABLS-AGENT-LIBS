cmake_minimum_required(VERSION 3.16)

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ABLS_AGENT_LIBS_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(NOT ABLS_AGENT_LIBS_VERSION)
  set(ABLS_AGENT_LIBS_VERSION "unknown")
endif()

if(ABLS_AGENT_LIBS_VERSION MATCHES "^v?([0-9]+\\.[0-9]+)")
  set(ABLS_AGENT_LIBS_VERSION_NUMBER "${CMAKE_MATCH_1}")
else()
  set(ABLS_AGENT_LIBS_VERSION_NUMBER "0.0")
endif()

project(abls-agent-libs
  VERSION ${ABLS_AGENT_LIBS_VERSION_NUMBER}
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

  set(CMAKE_REQUIRED_INCLUDES ${ABLS_LIBS_INCLUDE_DIRS})

add_library(abls-agent-libs SHARED
  src/agent.c
  src/http.c
  src/mqtt_local.c
  src/mnemonique.c
  src/dls.c
)

set_target_properties(abls-agent-libs PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "include/abls-agent-libs.h;include/agent.h;include/http.h;include/mqtt_local.h;include/mnemonique.h;include/dls.h"
)

target_include_directories(abls-agent-libs
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/abls-agent-libs>
  PRIVATE
    ${GLIB_INCLUDE_DIRS}
    ${GTHREAD_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${ABLS_LIBS_INCLUDE_DIRS}
)

target_compile_options(abls-agent-libs PRIVATE
  -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -pipe -g -ggdb
)

target_link_libraries(abls-agent-libs PRIVATE
  ${GLIB_LIBRARIES}
  ${GTHREAD_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
  ${ABLS_LIBS_LIBRARIES}
)

install(TARGETS abls-agent-libs
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/abls-agent-libs COMPONENT devel
)

configure_file(abls-agent-libs.pc.in abls-agent-libs.pc @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/abls-agent-libs.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT devel
)


set(CPACK_PACKAGE_NAME "abls-agent-libs")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_RELEASE "1")
set(CPACK_PACKAGE_VENDOR "Abls-Habitat")
set(CPACK_PACKAGE_CONTACT "sebastien.lefevre@abls-habitat.fr")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_LICENSE "GPLv3")

set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL runtime devel)

set(CPACK_RPM_RUNTIME_PACKAGE_NAME "abls-agent-libs")
set(CPACK_RPM_RUNTIME_PACKAGE_SUMMARY "Abls-Habitat satellite lifecycle library (runtime)")
set(CPACK_RPM_RUNTIME_PACKAGE_REQUIRES "glib2, json-glib, abls-libs")
set(CPACK_RPM_RUNTIME_POST_SCRIPTLET "/sbin/ldconfig")
set(CPACK_RPM_RUNTIME_FILE_NAME "abls-agent-libs-${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}.rpm")

set(CPACK_RPM_DEVEL_PACKAGE_NAME "abls-agent-libs-devel")
set(CPACK_RPM_DEVEL_PACKAGE_SUMMARY "Abls-Habitat satellite lifecycle library (development files)")
set(CPACK_RPM_DEVEL_PACKAGE_REQUIRES "abls-agent-libs = ${PROJECT_VERSION}, glib2-devel, json-glib-devel, abls-libs-devel")
set(CPACK_RPM_DEVEL_FILE_NAME "abls-agent-libs-devel-${PROJECT_VERSION}-${CPACK_PACKAGE_RELEASE}.rpm")

include(CPack)
