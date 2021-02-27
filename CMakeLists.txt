cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0054 NEW)

#Project's package support for non-UNIX platforms
include(lib/package/CMakeLists.txt)

#Project's metadata
project(mongoose)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#Build directory if not defined
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
endif()

#Project's source resolver
include(lib/source/CMakeLists.txt)

#Project's compile options
add_executable(mongoose ${SOURCE})
set_target_properties(mongoose PROPERTIES
		OUTPUT_NAME "mongoosed"
		CXX_STANDARD_REQUIRED 14
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION})
target_compile_definitions(mongoose PRIVATE -DTH_EXPORT)

#Project's compiler support for non-UNIX platforms
include(lib/compiler/CMakeLists.txt)

#Project's dependency resolvers
include(lib/external/tomahawk/CMakeLists.txt)