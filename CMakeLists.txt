cmake_minimum_required(VERSION 3.14)

project(mpv-inhibit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DBUS INTERFACE AND GENERATOR
# ======================================================================
add_library(offlrofl STATIC
	src/offlrofl/connection.cpp
	src/offlrofl/error.cpp
	src/offlrofl/message.cpp)
set_target_properties(offlrofl PROPERTIES POSITION_INDEPENDENT_CODE YES)
target_include_directories(offlrofl PUBLIC include)

find_package(PkgConfig)
pkg_check_modules(DBUS dbus-1)
target_include_directories(offlrofl PUBLIC ${DBUS_INCLUDE_DIRS})
target_link_libraries(offlrofl PUBLIC ${DBUS_LIBRARIES})

add_library(offlrofl::offlrofl ALIAS offlrofl)

add_executable(offlrofl_generate_interface
	src/offlrofl/generate_interface.cpp)
target_link_libraries(offlrofl_generate_interface offlrofl::offlrofl)

find_package(pugixml CONFIG REQUIRED)
target_link_libraries(offlrofl_generate_interface pugixml)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(offlrofl_generate_interface fmt)

add_executable(offlrofl::generate_interface ALIAS
	offlrofl_generate_interface)

# mpv-inhibit
# ======================================================================
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

set(THREADS_PREFER_PTHREAD_FLAG YES)
find_package(Threads REQUIRED)
target_link_libraries(mpv-inhibit Threads::Threads)

