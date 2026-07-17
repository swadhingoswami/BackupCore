add_library(backupcore_thread_pool
    thread_pool.h
    thread_pool.cpp
)
target_include_directories(backupcore_thread_pool PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_thread_pool PUBLIC backupcore_common Threads::Threads)
