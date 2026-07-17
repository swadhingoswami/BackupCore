find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_executable(test_backupcore
    test_main.cpp
    test_scanner.cpp
    test_chunker.cpp
    test_hasher.cpp
    test_repository.cpp
    test_manifest.cpp
    test_thread_pool.cpp
    test_restore.cpp
)

target_link_libraries(test_backupcore PRIVATE
    backupcore_engine
    backupcore_scanner
    backupcore_chunker
    backupcore_hasher
    backupcore_repository
    backupcore_manifest
    backupcore_thread_pool
    backupcore_common
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    OpenSSL::Crypto
    Threads::Threads
)

target_include_directories(test_backupcore PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME BackupCoreTests COMMAND test_backupcore)
