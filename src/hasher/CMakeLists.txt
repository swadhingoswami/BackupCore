add_library(backupcore_hasher
    hasher.h
    hasher.cpp
)
target_include_directories(backupcore_hasher PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(backupcore_hasher PUBLIC backupcore_common OpenSSL::Crypto)
