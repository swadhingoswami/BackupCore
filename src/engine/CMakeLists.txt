add_library(backupcore_engine
    backup_engine.h
    backup_engine.cpp
    restore_engine.h
    restore_engine.cpp
)
target_include_directories(backupcore_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_engine PUBLIC
    backupcore_scanner
    backupcore_chunker
    backupcore_hasher
    backupcore_repository
    backupcore_manifest
    backupcore_thread_pool
    backupcore_common
)
