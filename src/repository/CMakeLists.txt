add_library(backupcore_repository
    repository.h
    repository.cpp
)
target_include_directories(backupcore_repository PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_repository PUBLIC backupcore_common)
