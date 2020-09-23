#Resolve headers and linkage
if (NOT DEFINED TH_DIRECTORY)
    set(TH_DIRECTORY ${PROJECT_SOURCE_DIR}/../tomahawk CACHE STRING "Tomahawk's directory")
endif()

add_subdirectory(${TH_DIRECTORY} tomahawk)
link_directories(${TH_DIRECTORY})
target_include_directories(mongodb PRIVATE ${TH_DIRECTORY})
target_link_libraries(mongodb PRIVATE tomahawk)