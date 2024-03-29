cmake_minimum_required(VERSION 3.6)
if (POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
else()
	cmake_policy(SET CMP0054 NEW)
endif()

#Setup toolchain file if needed
set(TH_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../tomahawk CACHE STRING "Tomahawk directory")
set(TH_USE_ASSIMP OFF CACHE BOOL "Assimp is not used")
set(TH_USE_FREETYPE OFF CACHE BOOL "FreeType is not used")
set(TH_USE_GLEW OFF CACHE BOOL "GLEW is not used")
set(TH_USE_OPENAL OFF CACHE BOOL "OpenAL is not used")
set(TH_USE_OPENGL OFF CACHE BOOL "OpenGL is not used")
set(TH_USE_SDL2 OFF CACHE BOOL "SDL2 is not used")
set(TH_USE_POSTGRESQL OFF CACHE BOOL "PostgreSQL is not used")
set(TH_USE_MONGODB OFF CACHE BOOL "MongoDB is not used")
set(TH_USE_SPIRV OFF CACHE BOOL "SPIR-V and Glslang are not used")
set(TH_WITH_BULLET3 OFF CACHE BOOL "Bullet3 is not used")
set(TH_WITH_RMLUI OFF CACHE BOOL "RmlUi is not used")
set(TH_WITH_SHADERS OFF CACHE BOOL "Shaders are not used")
include(${TH_DIRECTORY}/lib/toolchain.cmake)

#Project's metadata
project(mongoose)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")

#Build directory if not defined
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
endif()

#Query all sources
file(GLOB_RECURSE SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx*
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx*)

#Resolving source groups
foreach(ITEM IN ITEMS ${SOURCE})
    get_filename_component(ITEM_PATH "${ITEM}" PATH)
    string(REPLACE "${PROJECT_SOURCE_DIR}" "" ITEM_GROUP "${ITEM_PATH}")
    string(REPLACE "/" "\\" ITEM_GROUP "${ITEM_GROUP}")
    source_group("${ITEM_GROUP}" FILES "${ITEM}")
endforeach()

#Project's compile options
add_executable(mongoose ${SOURCE})
set_target_properties(mongoose PROPERTIES
		OUTPUT_NAME "mongoosed"
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION})
target_compile_definitions(mongoose PRIVATE -DTH_EXPORT)

#Resolve needed options for different compilers and refs linkage
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    if (NOT (WIN32 AND MINGW))
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=0 -std=c++14 -fexceptions")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -fexceptions")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
			if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
				set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
			else()
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
				set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
			endif()
		endif()
	else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -D_WIN32_WINNT=0x0600")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows -D_WIN32_WINNT=0x0600")
    endif()
else()
    set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /MP")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG /MP")
    set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /MP")
    set(CMAKE_C_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG /MP")
    target_compile_definitions(mongoose PRIVATE
            -D_CRT_SECURE_NO_WARNINGS
            -D_SCL_SECURE_NO_WARNINGS)
endif()

#Project's dependency resolvers
include(lib/tomahawk/CMakeLists.txt)