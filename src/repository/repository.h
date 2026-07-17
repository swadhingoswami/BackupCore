#pragma once

#include "common/result.h"
#include "common/types.h"
#include <filesystem>
#include <unordered_set>
#include <mutex>

namespace backups {

class Repository {
public:
    explicit Repository(std::filesystem::path storage_path);

    Result<void> initialize();

    bool contains(const ChunkID& id) const;
    Result<void> store(const ChunkID& id, const ChunkData& data);
    Result<ChunkData> retrieve(const ChunkID& id) const;

    size_t unique_chunk_count() const;
    uint64_t total_stored_bytes() const;
    std::filesystem::path path() const { return storage_path_; }

private:
    std::filesystem::path storage_path_;
    std::filesystem::path chunks_path_;
    mutable std::mutex mutex_;
    std::unordered_set<ChunkID> chunks_;
    uint64_t total_bytes_ = 0;

    std::filesystem::path chunk_path(const ChunkID& id) const;
};

} // namespace backups
