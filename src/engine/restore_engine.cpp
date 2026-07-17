#include "restore_engine.h"
#include <iostream>
#include <fstream>
#include <system_error>
#include <iomanip>
#include <ctime>

namespace backups {

RestoreEngine::RestoreEngine(const Config& config)
    : config_(config)
    , repository_(std::make_unique<Repository>(config.repository_path))
    , manifest_(std::make_unique<Manifest>(config.repository_path / "manifests")) {}

Result<void> RestoreEngine::run_restore(
    const std::string& backup_id,
    const std::filesystem::path& output_dir)
{
    auto repo_result = repository_->initialize();
    if (repo_result.has_error()) {
        return repo_result;
    }

    auto load_result = manifest_->load(backup_id);
    if (load_result.has_error()) {
        return Result<void>(Error{load_result.error()});
    }

    auto manifest = std::move(load_result.value());

    std::cout << "Restoring: " << manifest.backup_id << "\n";
    std::cout << "Files: " << manifest.files.size() << "\n";

    std::error_code ec;
    if (!std::filesystem::exists(output_dir, ec)) {
        if (!std::filesystem::create_directories(output_dir, ec)) {
            return Result<void>(Error{std::string("Failed to create output dir: ") + ec.message()});
        }
    }

    size_t restored_files = 0;
    uint64_t restored_bytes = 0;

    for (const auto& fm : manifest.files) {
        auto output_path = output_dir / fm.path.relative_path();
        auto parent = output_path.parent_path();

        if (!std::filesystem::exists(parent, ec)) {
            if (!std::filesystem::create_directories(parent, ec)) {
                std::cerr << "  Warning: Cannot create directory "
                          << parent << ": " << ec.message() << "\n";
                continue;
            }
        }

        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "  Warning: Cannot create " << output_path << "\n";
            continue;
        }

        bool file_ok = true;
        for (const auto& chunk : fm.chunks) {
            auto chunk_result = repository_->retrieve(chunk.id);
            if (chunk_result.has_error()) {
                std::cerr << "  Warning: Missing chunk " << chunk.id
                          << " for " << fm.path << "\n";
                file_ok = false;
                break;
            }

            auto& data = chunk_result.value();
            out_file.write(reinterpret_cast<const char*>(data.data()),
                           static_cast<std::streamsize>(data.size()));
            if (!out_file.good()) {
                std::cerr << "  Warning: Write error for " << output_path << "\n";
                file_ok = false;
                break;
            }
        }

        if (file_ok) {
            restored_files++;
            restored_bytes += fm.original_size;
            std::cout << "  Restored: " << fm.path << "\n";
        }
    }

    std::cout << "Restore complete.\n";
    std::cout << "  Files restored: " << restored_files << "/" << manifest.files.size() << "\n";
    std::cout << "  Bytes restored: " << restored_bytes << "\n";

    return Result<void>();
}

Result<void> RestoreEngine::list_backups() {
    auto repo_result = repository_->initialize();
    if (repo_result.has_error()) {
        return Result<void>(Error{repo_result.error()});
    }

    auto list_result = manifest_->list_backups();
    if (list_result.has_error()) {
        return Result<void>(Error{list_result.error()});
    }

    auto ids = std::move(list_result.value());

    if (ids.empty()) {
        std::cout << "No backups found.\n";
        return Result<void>();
    }

    std::cout << "Available backups:\n";
    for (const auto& id : ids) {
        auto load_result = manifest_->load(id);
        if (load_result.has_value()) {
            auto tt = std::chrono::system_clock::to_time_t(load_result.value().timestamp);
            std::cout << "  " << id
                      << "  (" << std::put_time(std::gmtime(&tt), "%Y-%m-%d %H:%M:%S")
                      << ", " << load_result.value().files.size() << " files)\n";
        }
    }

    return Result<void>();
}

} // namespace backups
