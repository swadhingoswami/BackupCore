#pragma once

#include "common/result.h"
#include "common/types.h"
#include <vector>

namespace backups {

class Chunker {
public:
    explicit Chunker(size_t chunk_size);

    struct Chunk {
        size_t index;
        uint64_t offset;
        ChunkData data;
    };

    Result<std::vector<Chunk>> chunk(const ChunkData& data);

    size_t chunk_size() const { return chunk_size_; }

private:
    size_t chunk_size_;
};

} // namespace backups
