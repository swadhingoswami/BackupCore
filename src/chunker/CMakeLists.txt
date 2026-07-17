add_library(backupcore_chunker
    chunker.h
    chunker.cpp
)
target_include_directories(backupcore_chunker PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_chunker PUBLIC backupcore_common)
