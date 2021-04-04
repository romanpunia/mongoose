#Resolve headers and linkage
add_subdirectory(${TH_DIRECTORY} tomahawk)
link_directories(${TH_DIRECTORY})
target_include_directories(mongoose PRIVATE ${TH_DIRECTORY})
target_link_libraries(mongoose PRIVATE tomahawk)