#include "repository.h"
#include <fstream>
#include <system_error>

namespace backups {

Repository::Repository(std::filesystem::path storage_path)
    : storage_path_(std::move(storage_path))
    , chunks_path_(storage_path_ / "chunks") {}

Result<void> Repository::initialize() {
    std::error_code ec;

    if (!std::filesystem::exists(storage_path_, ec)) {
        if (!std::filesystem::create_directories(storage_path_, ec)) {
            return Result<void>(Error{std::string("Failed to create repository: ") + ec.message()});
        }
    }

    if (!std::filesystem::exists(chunks_path_, ec)) {
        if (!std::filesystem::create_directories(chunks_path_, ec)) {
            return Result<void>(Error{std::string("Failed to create chunks dir: ") + ec.message()});
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(chunks_path_, ec)) {
        if (entry.is_regular_file()) {
            ChunkID id = entry.path().stem().string();
            chunks_.insert(id);
            total_bytes_ += static_cast<uint64_t>(entry.file_size());
        }
    }

    if (ec) {
        return Result<void>(Error{std::string("Failed to scan existing chunks: ") + ec.message()});
    }

    return Result<void>();
}

bool Repository::contains(const ChunkID& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return chunks_.contains(id);
}

Result<void> Repository::store(const ChunkID& id, const ChunkData& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (chunks_.contains(id)) {
        return Result<void>();
    }

    auto path = chunk_path(id);

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return Result<void>(Error{std::string("Failed to open chunk file: ") + path.string()});
    }

    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));

    if (!file.good()) {
        return Result<void>(Error{std::string("Failed to write chunk: ") + path.string()});
    }

    chunks_.insert(id);
    total_bytes_ += data.size();

    return Result<void>();
}

Result<ChunkData> Repository::retrieve(const ChunkID& id) const {
    auto path = chunk_path(id);

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return Result<ChunkData>(Error{std::string("Chunk not found: ") + id});
    }

    auto size = static_cast<size_t>(file.tellg());
    file.seekg(0);

    ChunkData data(size);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));

    if (!file.good()) {
        return Result<ChunkData>(Error{std::string("Failed to read chunk: ") + id});
    }

    return Result<ChunkData>(std::move(data));
}

size_t Repository::unique_chunk_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return chunks_.size();
}

uint64_t Repository::total_stored_bytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return total_bytes_;
}

std::filesystem::path Repository::chunk_path(const ChunkID& id) const {
    return chunks_path_ / (id + ".chunk");
}

} // namespace backups
