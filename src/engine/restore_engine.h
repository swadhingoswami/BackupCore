#pragma once

#include "common/config.h"
#include "common/result.h"
#include "common/types.h"
#include "repository/repository.h"
#include "manifest/manifest.h"

#include <memory>

namespace backups {

class RestoreEngine {
public:
    explicit RestoreEngine(const Config& config);

    Result<void> run_restore(
        const std::string& backup_id,
        const std::filesystem::path& output_dir);

    Result<void> list_backups();

private:
    Config config_;
    std::unique_ptr<Repository> repository_;
    std::unique_ptr<Manifest> manifest_;
};

} // namespace backups
