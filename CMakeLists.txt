cmake_minimum_required(VERSION 3.14)

project(mpv-inhibit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../offlrofl offlrofl)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/screensaver_interface.h
	COMMAND offlrofl::generate_interface org.freedesktop.ScreenSaver > ${CMAKE_CURRENT_BINARY_DIR}/screensaver_interface.h
	DEPENDS offlrofl::generate_interface
	VERBATIM)

add_library(mpv-inhibit MODULE
	src/inhibit.cpp
	${CMAKE_CURRENT_BINARY_DIR}/screensaver_interface.h)

target_include_directories(mpv-inhibit PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(mpv-inhibit offlrofl::offlrofl)

find_file(MPV_INCLUDE_DIR mpv/client.h)
if(NOT MPV_INCLUDE_DIR)
	message(FATAL_ERROR "Cannot find libmpv headers. Are you missing a libmpv-dev package?")
endif()
target_include_directories(mpv-inhibit PRIVATE ${MPV_INCLUDE_DIR})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(mpv-inhibit fmt::fmt)

