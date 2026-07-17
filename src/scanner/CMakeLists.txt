add_library(backupcore_scanner
    scanner.h
    scanner.cpp
)
target_include_directories(backupcore_scanner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_scanner PUBLIC backupcore_common)
