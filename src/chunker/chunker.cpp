#include "chunker.h"
#include <algorithm>
#include <cstring>

namespace backups {

Chunker::Chunker(size_t chunk_size) : chunk_size_(chunk_size) {}

Result<std::vector<Chunker::Chunk>> Chunker::chunk(const ChunkData& data) {
    std::vector<Chunk> chunks;
    size_t offset = 0;

    while (offset < data.size()) {
        size_t remaining = data.size() - offset;
        size_t this_chunk_size = std::min(chunk_size_, remaining);

        Chunk chunk;
        chunk.index = chunks.size();
        chunk.offset = static_cast<uint64_t>(offset);
        chunk.data.resize(this_chunk_size);

        std::memcpy(chunk.data.data(), data.data() + offset, this_chunk_size);

        chunks.push_back(std::move(chunk));
        offset += this_chunk_size;
    }

    return Result<std::vector<Chunk>>(std::move(chunks));
}

} // namespace backups
