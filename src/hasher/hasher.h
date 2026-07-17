#pragma once

#include "common/result.h"
#include "common/types.h"
#include <string>
#include <span>

namespace backups {

class Hasher {
public:
    Hasher() = default;

    Result<ChunkID> hash(const ChunkData& data);

    static std::string bytes_to_hex(std::span<const unsigned char> bytes);

private:
};

} // namespace backups
