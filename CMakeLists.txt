cmake_minimum_required(VERSION 2.8)
project(ibus-fwnn C)

set(IBUS_COMPONENT_DIR "/usr/share/ibus/component/")
set(IBUS_FWNN_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/share/ibus-fwnn")
set(IBUS_FWNN_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/ibus-fwnn")

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")


configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)
configure_file(${PROJECT_SOURCE_DIR}/fwnn.xml.in ${PROJECT_SOURCE_DIR}/fwnn.xml)

# check ibus
find_package(PkgConfig REQUIRED)
pkg_check_modules(IBUS REQUIRED ibus-1.0)
pkg_check_modules(WNN REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)

install(FILES ${PROJECT_SOURCE_DIR}/fwnn.xml DESTINATION ${IBUS_COMPONENT_DIR})
