#pragma once

#include "common/result.h"
#include "common/types.h"
#include <filesystem>
#include <vector>

namespace backups {

class Scanner {
public:
    explicit Scanner(const std::filesystem::path& root);

    Result<std::vector<FileInfo>> scan();

private:
    std::filesystem::path root_;
};

} // namespace backups
