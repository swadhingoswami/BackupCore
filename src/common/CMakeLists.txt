add_library(backupcore_common
    types.h
    result.h
    config.h
    config.cpp
)
target_include_directories(backupcore_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
