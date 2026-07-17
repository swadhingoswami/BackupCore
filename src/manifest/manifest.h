#pragma once

#include "common/result.h"
#include "common/types.h"
#include <filesystem>
#include <string>
#include <vector>

namespace backups {

class Manifest {
public:
    explicit Manifest(std::filesystem::path manifest_dir);

    Result<void> save(const BackupManifest& manifest);
    Result<BackupManifest> load(const std::string& backup_id) const;

    Result<std::vector<std::string>> list_backups() const;

    static std::string generate_backup_id();

private:
    std::filesystem::path manifest_dir_;

    std::filesystem::path manifest_path(const std::string& backup_id) const;
};

} // namespace backups
