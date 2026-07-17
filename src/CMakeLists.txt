add_subdirectory(common)
add_subdirectory(scanner)
add_subdirectory(chunker)
add_subdirectory(hasher)
add_subdirectory(repository)
add_subdirectory(manifest)
add_subdirectory(thread_pool)
add_subdirectory(engine)
add_subdirectory(cli)

add_executable(backupcore main.cpp)
target_link_libraries(backupcore PRIVATE
    backupcore_cli
    backupcore_engine
    backupcore_scanner
    backupcore_chunker
    backupcore_hasher
    backupcore_repository
    backupcore_manifest
    backupcore_thread_pool
    backupcore_common
    OpenSSL::Crypto
    Threads::Threads
)
