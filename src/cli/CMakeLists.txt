add_library(backupcore_cli
    cli.h
    cli.cpp
)
target_include_directories(backupcore_cli PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_cli PUBLIC backupcore_engine backupcore_common)
