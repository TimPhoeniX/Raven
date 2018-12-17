cmake_minimum_required(VERSION 3.7)
project(Raven)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

include(CMakeModules/sge_debug_info.cmake)

#option(PACKAGE_MANAGER "Default project package manager default: NONE" NONE)
#option(SGE_ROOT "SGE install directory" NONE)
option(INCLUDE_ENGINE_PROJECT "If enabled includes SGE as subproject" OFF)
option(COPY_SHADERS "If the SGE was not included as sub-project settig this option will copy GLSL files into CMAKE_RUNTIME_OUTPUT_DIRECTORY" OFF)
option(COPY_GAME_RESOURCES "If enabled copies game resources into CMAKE_RUNTIME_OUTPUT_DIRECTORY" OFF)
#option(SETUP_BUILD_DIR "Setup project libs path" NONE)
#option(CMAKE_TOOLCHAIN_FILE "CMake toolchain file defines build configuration" NONE)

set(SETUP_BUILD_DIR NONE CACHE PATH "Setup project libs path")
set(SGE_ROOT NONE CACHE PATH "SGE install directory")
set(CMAKE_INSTALL_PREFIX NONE CACHE PATH "Defines to path in which SGE framework should be installed, default: NONE")
set(CMAKE_TOOLCHAIN_FILE NONE CACHE FILEPATH "CMake toolchain file defines build configuration")
set(PACKAGE_MANAGER NONE CACHE STRING "Default project package manager, default: NONE")

set(MASTER_BUILD_TYPES "Debug Release RelWithDebInfo MinSizeRel")
string(REPLACE " " ";" MASTER_BUILD_TYPES_AVAILABLE ${MASTER_BUILD_TYPES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build -- release is default. Other: ${MASTER_BUILD_TYPES}.")
	message(STATUS "Setting default CMake build type: ${CMAKE_BUILD_TYPE}")
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build -- release is default. Other: ${MASTER_BUILD_TYPES}.")
	message(STATUS "Setting default CMake build type: ${CMAKE_BUILD_TYPE}")
else()
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build -- release is default. Other: ${MASTER_BUILD_TYPES}.")
	message(STATUS "Provided CMake build type: ${CMAKE_BUILD_TYPE}")
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${MASTER_BUILD_TYPES_AVAILABLE})

if (NOT PACKAGE_MANAGER)
	message(WARNING "Option PACKAGE_MANAGER not passed, setting: NONE")
	set(PACKAGE_MANAGER "NONE")
endif()

set(PKG_TYPES "None Conan Setup")
string(REPLACE " " ";" PKG_TYPES_AVAILABLE ${PKG_TYPES})
set_property(CACHE PACKAGE_MANAGER PROPERTY STRINGS ${PKG_TYPES_AVAILABLE})

if(${PACKAGE_MANAGER} STREQUAL "Setup")
	if (NOT SETUP_BUILD_DIR)
		message(WARNING "Option SETUP_BUILD_DIR has not been provided. Using default value: Setup/build")
		set(SETUP_BUILD_DIR "Setup/build")
	endif()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

message(STATUS "BUILD TYPE: ${CMAKE_BUILD_TYPE}")

if (BUILD_SHARED_LIBS AND WIN32)
	message(STATUS "BUILD_SHARED_LIBS is ON. Adding compiler defintion.")
	add_definitions(-DSGE_SHARED)
endif()

if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        message(STATUS "Linux")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fext-numeric-literals -pthread -ldl")
        set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fext-numeric-literals -pthread -ldl")
    
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        message(STATUS "Darwin")

        set(CMAKE_MACOSX_RPATH OFF)
	endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O3")
else()
	message (STATUS "MSVC ${MSVC_VERSION}v")

	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /W2")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2")

	set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /W2")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /O2")

	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Raven)
	link_directories("${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()

message(STATUS "Using PACKAGE_MANAGER: ${PACKAGE_MANAGER}")

if (${PACKAGE_MANAGER} STREQUAL "Conan")
	message(STATUS "Using PACKAGE_MANAGER: Conan")

	include(${CMAKE_CURRENT_SOURCE_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()

	include(CMakeModules/sge_common_find.cmake)
elseif(${PACKAGE_MANAGER} STREQUAL "Setup")
	message(STATUS "SETUP_BUILD_DIR: ${SETUP_BUILD_DIR}")

	include(CMakeModules/sge_setup.cmake)
else()
	message(STATUS "Using PACKAGE_MANAGER: None")
	message(STATUS "No ROOT for libs will be set up")
	message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

	include(CMakeModules/sge_common_find.cmake)
endif()

find_package(Boost COMPONENTS date_time program_options system filesystem REQUIRED)
if(NOT Boost_FOUND)
	message(Error "Boost not found")
else()
	include_directories(${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
endif()

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
	message(Error "SDL2 not found")
else()
	#include_directories("${SDL2_INCLUDE_DIR}")
	#include_directories("${SDL2_INCLUDE_DIR}/SDL2")
	#link_directories("${SDL2_DIR}/lib")
	#add_definitions(${SDL2_DEFINITIONS})
endif()

find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
        message(Error "OpenGL not found")
else()
	include_directories(${OpenGL_INCLUDE_DIRS})
	link_directories(${OpenGL_LIBRARY_DIRS})
	add_definitions(${OpenGL_DEFINITIONS})
endif()

include(CMakeModules/sge_status.cmake)

set(SGE_DIR "SGE/SimpleGameEngine")

if (INCLUDE_ENGINE_PROJECT)
	if (WIN32 AND BUILD_SHARED_LIBS)
		include_directories(${CMAKE_BINARY_DIR}/exports)
	endif()
	include_directories(${SGE_DIR})
	add_subdirectory(${SGE_DIR})
else()
	if (NOT SGE_ROOT)
		message(WARNING "SGE_ROOT missing!")
	endif()

	find_package(SGE REQUIRED)
	if(NOT SGE_FOUND)
		message(Error "SGE not found")
	else()
		include_directories(${SGE_INCLUDE_DIR})
		link_directories(${SGE_LIBRARY_DIR})
		message("SGE_INCLUDE_DIR: ${SGE_INCLUDE_DIR}")
		message("SGE_LIBRARY_DIR: ${SGE_LIBRARY_DIR}")
		message("SGE_LIBRARIES: ${SGE_LIBRARIES}")
	endif()
endif()

add_subdirectory(Raven)
