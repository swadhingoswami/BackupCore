add_library(backupcore_manifest
    manifest.h
    manifest.cpp
)
target_include_directories(backupcore_manifest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_manifest PUBLIC backupcore_common)
