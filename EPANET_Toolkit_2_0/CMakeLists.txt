# CMakeLists.txt - CMake configuration file for EPANET 2.0
#
# CMake is a cross-platform build tool. CMake generates platform native
# build systems that can be used with your compiler of choice. CMake uses a
# generator concept to represent different build tooling. CMake automatically
# detects the platform it is running on and generates the appropriate makefiles
# for the platform default compiler. Different generators can also be specified.
#
# Note: CMake requires that your platform build system and compiler are
# properly installed. Build using Visual Studio requires msbuild shell.
#
# Build Options:
#   BUILD_TESTS = ON/OFF
#   BUILD_PY_LIB = ON/OFF
#
# Generic Invocation:
#   cmake -E make_directory buildprod
#   cd build
#   cmake -G GENERATOR -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
#   cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
#
#   cmake --build . --target SOME_TARGET --config Release
#   cmake --build . --target epanet2 --config Release
#   cmake --build . --target runepanet --config Release
#   ./bin/runepanet ./bin/vanZyl.inp ./bin/schedules0
#
# More information:
#   cmake --help
#
#   CMake is available at https://cmake.org/download/
#

cmake_minimum_required (VERSION 2.8.8)

# Name of the project
project(EPANET_Tookit)

# ------------ Code coverage module ------------
# List operation to append local dir to module search path
# CMAKE_MODULE_PATH is a semicolon-separated list of directories specifying a search path for CMake modules
# Removed
# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ------------ Provide an option that the user can optionally select ------------
option(BUILD_TESTS "Build tests (requires Boost)" OFF)
option(BUILD_PY_LIB "Build library for Python wrapper" OFF)
option(BUILD_COVERAGE "Build library for coverage" OFF)

# ------------ Add stub for a command line executable version of EPANET ------------
IF (NOT BUILD_PY_LIB)
	add_subdirectory(run)
ENDIF (NOT BUILD_PY_LIB)

# ------------ Add epanet-output library ------------
# API for reading results from EPANET binary output file
# Add a subdirectory to the build.
# Removed
# add_subdirectory(src/outfile)

IF (BUILD_TESTS)
	# Prep ourselves for compiling with boost
	IF (WIN32)
		set(Boost_USE_STATIC_LIBS       ON)
  ELSE (TRUE)
		set(Boost_USE_STATIC_LIBS       OFF)
		add_definitions(-DBOOST_ALL_DYN_LINK)
	ENDIF (WIN32)

  find_package(Boost COMPONENTS unit_test_framework system thread filesystem)
  include_directories (${Boost_INCLUDE_DIRS})

  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(tests/outfile)
  add_subdirectory(tests/util)
ENDIF (BUILD_TESTS)

# ------------ Sets the output directory for executables and libraries ------------
# Output directory in which to build RUNTIME target files.
# A runtime output artifact of a buildsystem target may be:
# - The executable file (e.g. .exe) of an executable target created by the add_executable() command.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Where to put all the LIBRARY target files when built.
# Output directory in which to build LIBRARY target files.
# A library output artifact of a buildsystem target may be:
# - The loadable module file (e.g. .dll or .so) of a module library target created by the add_library() command with the MODULE option.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Output directory in which to build ARCHIVE target files.
# An archive output artifact of a buildsystem target may be:
# - The static library file (e.g. .lib or .a) of a static library target created by the add_library() command with the STATIC option.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ------------ Sets the position independent code property for all targets ------------
# Explanation: https://alexsm.com/fpic-cmake-pybind11/
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF (APPLE)
	set(INSTALL_NAME_DIR @executable_path/../lib)
  set(CMAKE_MACOSX_RPATH 1)
ENDIF (APPLE)

IF (MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/GL")
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -MT)
ENDIF (MSVC)

# ------------ Configure file groups ------------
# This command is dedicated to file and path manipulation requiring access to the filesystem.
# Generate a list of files that match the <globbing-expressions> and store it into the <variable>.
file(GLOB EPANET_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} src/*.c src/util/*.c)
file(GLOB EPANET_LIB_ALL RELATIVE ${PROJECT_SOURCE_DIR} src/* src/util/*)
# Exclude epanet python API from the default build.
list(REMOVE_ITEM EPANET_LIB_ALL "src/epanet_py.c")

# ------------ Defines a group into which sources will be placed in project files ------------
source_group("Library" FILES ${EPANET_LIB_ALL})
# the shared library
IF(MSVC AND "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(Win32)")
  message("   ************************************")
  message("   Configuring with epanet2.def mapping")
  message("   ************************************")
  add_library(epanet2 SHARED ${EPANET_LIB_ALL} ${PROJECT_SOURCE_DIR}/include/epanet2.def)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/include/epanet2.def PROPERTIES_HEADER_FILE_ONLY TRUE)
ELSE(TRUE)
  add_library(epanet2 SHARED ${EPANET_LIB_ALL})
ENDIF(MSVC AND "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(Win32)")


# ------------ Specifies include directories to use when compiling a given target ------------
target_include_directories(epanet2 PUBLIC ${PROJECT_SOURCE_DIR}/include)
