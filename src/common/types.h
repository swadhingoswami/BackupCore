#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <cstddef>

namespace backups {

using ChunkID = std::string;
using ChunkData = std::vector<std::byte>;

struct FileInfo {
    std::filesystem::path path;
    uint64_t size = 0;
};

struct ChunkRecord {
    ChunkID id;
    size_t size = 0;
    uint64_t offset = 0;
};

struct FileManifest {
    std::filesystem::path path;
    uint64_t original_size = 0;
    std::vector<ChunkRecord> chunks;
};

struct BackupManifest {
    std::string backup_id;
    std::chrono::system_clock::time_point timestamp;
    std::vector<FileManifest> files;
};

struct Config {
    size_t chunk_size = 4 * 1024 * 1024;
    int worker_threads = 4;
    std::filesystem::path repository_path = "./repository";
};

} // namespace backups
