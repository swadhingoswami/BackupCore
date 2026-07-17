#include "backup_engine.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

namespace backups {

BackupEngine::BackupEngine(const Config& config)
    : config_(config)
    , repository_(std::make_unique<Repository>(config.repository_path))
    , manifest_(std::make_unique<Manifest>(config.repository_path / "manifests"))
    , pool_(std::make_unique<ThreadPool>(static_cast<size_t>(config.worker_threads))) {}

Result<std::string> BackupEngine::run_backup(const std::filesystem::path& source_dir) {
    auto repo_result = repository_->initialize();
    if (repo_result.has_error()) {
        return Result<std::string>(Error{repo_result.error()});
    }

    Scanner scanner(source_dir);
    auto scan_result = scanner.scan();
    if (scan_result.has_error()) {
        return Result<std::string>(Error{scan_result.error()});
    }

    auto files = std::move(scan_result.value());
    if (files.empty()) {
        return Result<std::string>(Error{std::string("No files found in: ") + source_dir.string()});
    }

    current_manifest_.backup_id = Manifest::generate_backup_id();
    current_manifest_.timestamp = std::chrono::system_clock::now();
    source_dir_ = source_dir;

    std::cout << "Backup: " << current_manifest_.backup_id << "\n";
    std::cout << "Files: " << files.size() << "\n";

    size_t total_size = 0;
    for (const auto& f : files) {
        total_size += f.size;
    }
    std::cout << "Total size: " << total_size << " bytes\n";

    std::atomic<size_t> files_processed{0};
    size_t total_files = files.size();

    for (const auto& file : files) {
        pool_->enqueue([this, file, &files_processed, total_files]() {
            process_file(file.path, file.size);
            size_t done = ++files_processed;
            if (done % 100 == 0 || done == total_files) {
                std::cout << "  Progress: " << done << "/" << total_files
                          << " files  (unique chunks: "
                          << repository_->unique_chunk_count() << ")\n";
            }
        });
    }

    pool_->wait_all();

    auto save_result = manifest_->save(current_manifest_);
    if (save_result.has_error()) {
        return Result<std::string>(Error{save_result.error()});
    }

    std::cout << "Backup complete.\n";
    std::cout << "  Files backed up: " << files.size() << "\n";
    std::cout << "  Unique chunks: " << repository_->unique_chunk_count() << "\n";
    std::cout << "  Total stored: " << repository_->total_stored_bytes() << " bytes\n";
    std::cout << "  Dedup ratio: ";
    if (total_size > 0) {
        double ratio = static_cast<double>(total_size) /
                       std::max(static_cast<double>(repository_->total_stored_bytes()), 1.0);
        std::cout << ratio << "x\n";
    } else {
        std::cout << "N/A\n";
    }

    return Result<std::string>(current_manifest_.backup_id);
}

void BackupEngine::process_file(const std::filesystem::path& file_path, uint64_t file_size) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "  Warning: Cannot open " << file_path << "\n";
        return;
    }

    FileManifest file_manifest;
    file_manifest.path = std::filesystem::relative(file_path, source_dir_);
    file_manifest.original_size = file_size;

    ChunkData buffer(config_.chunk_size);
    uint64_t offset = 0;
    size_t chunk_index = 0;

    while (file.good() && offset < file_size) {
        size_t to_read = std::min(config_.chunk_size, static_cast<size_t>(file_size - offset));
        file.read(reinterpret_cast<char*>(buffer.data()),
                  static_cast<std::streamsize>(to_read));
        size_t bytes_read = static_cast<size_t>(file.gcount());

        if (bytes_read == 0) break;

        ChunkData chunk_data(buffer.begin(), buffer.begin() + static_cast<ptrdiff_t>(bytes_read));

        Hasher hasher;
        auto hash_result = hasher.hash(chunk_data);
        if (hash_result.has_error()) {
            std::cerr << "  Warning: Hash failed for " << file_path << ": "
                      << hash_result.error() << "\n";
            offset += bytes_read;
            chunk_index++;
            continue;
        }

        ChunkID chunk_id = std::move(hash_result.value());

        bool is_new = !repository_->contains(chunk_id);
        auto store_result = repository_->store(chunk_id, chunk_data);
        if (store_result.has_error()) {
            std::cerr << "  Warning: Store failed for " << file_path << ": "
                      << store_result.error() << "\n";
        }

        ChunkRecord record;
        record.id = std::move(chunk_id);
        record.size = bytes_read;
        record.offset = offset;
        file_manifest.chunks.push_back(std::move(record));

        offset += bytes_read;
        chunk_index++;
    }

    {
        std::lock_guard<std::mutex> lock(manifest_mutex_);
        current_manifest_.files.push_back(std::move(file_manifest));
    }
}

} // namespace backups
