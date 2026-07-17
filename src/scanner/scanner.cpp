#include "scanner.h"
#include <system_error>

namespace backups {

Scanner::Scanner(const std::filesystem::path& root) : root_(root) {}

Result<std::vector<FileInfo>> Scanner::scan() {
    std::vector<FileInfo> files;

    std::error_code ec;
    if (!std::filesystem::exists(root_, ec)) {
        return Result<std::vector<FileInfo>>(Error{std::string("Path does not exist: ") + root_.string()});
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root_, ec)) {
        if (ec) {
            return Result<std::vector<FileInfo>>(Error{std::string("Error scanning: ") + ec.message()});
        }

        if (entry.is_regular_file()) {
            FileInfo info;
            info.path = entry.path();
            info.size = static_cast<uint64_t>(entry.file_size());
            files.push_back(std::move(info));
        }
    }

    if (ec) {
        return Result<std::vector<FileInfo>>(Error{std::string("Scan failed: ") + ec.message()});
    }

    return Result<std::vector<FileInfo>>(std::move(files));
}

} // namespace backups
