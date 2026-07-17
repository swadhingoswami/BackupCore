#pragma once

#include "common/config.h"
#include "common/result.h"
#include "common/types.h"
#include "scanner/scanner.h"
#include "chunker/chunker.h"
#include "hasher/hasher.h"
#include "repository/repository.h"
#include "manifest/manifest.h"
#include "thread_pool/thread_pool.h"

#include <memory>
#include <atomic>
#include <mutex>

namespace backups {

class BackupEngine {
public:
    explicit BackupEngine(const Config& config);

    Result<std::string> run_backup(const std::filesystem::path& source_dir);

private:
    Config config_;
    std::unique_ptr<Repository> repository_;
    std::unique_ptr<Manifest> manifest_;
    std::unique_ptr<ThreadPool> pool_;
    std::filesystem::path source_dir_;

    BackupManifest current_manifest_;
    std::mutex manifest_mutex_;

    void process_file(const std::filesystem::path& file_path, uint64_t file_size);
    void process_job(const std::filesystem::path& file_path, uint64_t file_size);
};

} // namespace backups
