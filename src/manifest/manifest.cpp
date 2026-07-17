#include "manifest.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <system_error>
#include <algorithm>

namespace backups {

Manifest::Manifest(std::filesystem::path manifest_dir)
    : manifest_dir_(std::move(manifest_dir)) {}

std::string Manifest::generate_backup_id() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << "backup_"
        << std::put_time(std::gmtime(&tt), "%Y%m%d_%H%M%S")
        << "_" << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

static std::string escape_json(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out += c;
        }
    }
    return out;
}

static void write_indent(std::ostream& os, int indent) {
    for (int i = 0; i < indent; i++) os << "  ";
}

static void write_file_manifest(std::ostream& os, const FileManifest& fm, int indent) {
    write_indent(os, indent); os << "{\n";
    write_indent(os, indent + 1); os << "\"path\": \"" << escape_json(fm.path.generic_string()) << "\",\n";
    write_indent(os, indent + 1); os << "\"original_size\": " << fm.original_size << ",\n";
    write_indent(os, indent + 1); os << "\"chunks\": [\n";

    for (size_t i = 0; i < fm.chunks.size(); i++) {
        const auto& cr = fm.chunks[i];
        write_indent(os, indent + 2); os << "{\n";
        write_indent(os, indent + 3); os << "\"id\": \"" << cr.id << "\",\n";
        write_indent(os, indent + 3); os << "\"size\": " << cr.size << ",\n";
        write_indent(os, indent + 3); os << "\"offset\": " << cr.offset << "\n";
        write_indent(os, indent + 2); os << "}";
        if (i + 1 < fm.chunks.size()) os << ",";
        os << "\n";
    }

    write_indent(os, indent + 1); os << "]\n";
    write_indent(os, indent); os << "}";
}

Result<void> Manifest::save(const BackupManifest& manifest) {
    std::error_code ec;
    if (!std::filesystem::exists(manifest_dir_, ec)) {
        if (!std::filesystem::create_directories(manifest_dir_, ec)) {
            return Result<void>(Error{std::string("Failed to create manifest dir: ") + ec.message()});
        }
    }

    auto path = manifest_path(manifest.backup_id);
    std::ofstream file(path);
    if (!file.is_open()) {
        return Result<void>(Error{std::string("Failed to write manifest: ") + path.string()});
    }

    auto tt = std::chrono::system_clock::to_time_t(manifest.timestamp);

    file << "{\n";
    file << "  \"backup_id\": \"" << escape_json(manifest.backup_id) << "\",\n";
    file << "  \"timestamp\": " << tt << ",\n";
    file << "  \"timestamp_iso\": \"" << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ") << "\",\n";
    file << "  \"files\": [\n";

    for (size_t i = 0; i < manifest.files.size(); i++) {
        write_file_manifest(file, manifest.files[i], 2);
        if (i + 1 < manifest.files.size()) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    if (!file.good()) {
        return Result<void>(Error{std::string("Failed to write manifest data")});
    }

    return Result<void>();
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string extract_string(const std::string& key, const std::string& line) {
    auto pos = line.find(key);
    if (pos == std::string::npos) return "";

    auto colon = line.find(':', pos);
    if (colon == std::string::npos) return "";

    auto quote_start = line.find('"', colon);
    if (quote_start == std::string::npos) return "";

    auto quote_end = line.find('"', quote_start + 1);
    if (quote_end == std::string::npos) return "";

    return line.substr(quote_start + 1, quote_end - quote_start - 1);
}

static int64_t extract_integer(const std::string& key, const std::string& line) {
    auto pos = line.find(key);
    if (pos == std::string::npos) return 0;

    auto colon = line.find(':', pos);
    if (colon == std::string::npos) return 0;

    auto after = line.substr(colon + 1);
    auto trimmed = trim(after);

    size_t end = 0;
    int64_t val = std::stoll(trimmed, &end);
    return val;
}

static bool line_contains(const std::string& line, const std::string& s) {
    return line.find(s) != std::string::npos;
}

static std::vector<std::string> read_lines(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return {};

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

Result<BackupManifest> Manifest::load(const std::string& backup_id) const {
    auto path = manifest_path(backup_id);
    auto lines = read_lines(path);

    if (lines.empty()) {
        return Result<BackupManifest>(Error{std::string("Manifest not found: ") + backup_id});
    }

    BackupManifest manifest;
    manifest.backup_id = backup_id;

    for (size_t i = 0; i < lines.size(); i++) {
        const auto& line = lines[i];

        if (line_contains(line, "backup_id")) {
            auto id = extract_string("backup_id", line);
            if (!id.empty()) manifest.backup_id = id;
        }

        if (line_contains(line, "timestamp") && !line_contains(line, "timestamp_iso")) {
            auto ts = extract_integer("timestamp", line);
            manifest.timestamp = std::chrono::system_clock::from_time_t(ts);
        }

        if (line_contains(line, "\"path\"")) {
            FileManifest fm;
            fm.path = extract_string("path", line);
            manifest.files.push_back(std::move(fm));
        }

        if (line_contains(line, "original_size")) {
            if (!manifest.files.empty()) {
                manifest.files.back().original_size = static_cast<uint64_t>(extract_integer("original_size", line));
            }
        }

        if (line_contains(line, "\"id\"")) {
            ChunkRecord cr;
            cr.id = extract_string("id", line);
            manifest.files.back().chunks.push_back(std::move(cr));
        }

        if (line_contains(line, "\"size\"") && line_contains(line, "\"id\"") == false) {
            if (!manifest.files.empty() && !manifest.files.back().chunks.empty()) {
                manifest.files.back().chunks.back().size = static_cast<size_t>(extract_integer("size", line));
            }
        }

        if (line_contains(line, "\"offset\"")) {
            if (!manifest.files.empty() && !manifest.files.back().chunks.empty()) {
                manifest.files.back().chunks.back().offset = static_cast<uint64_t>(extract_integer("offset", line));
            }
        }
    }

    return Result<BackupManifest>(std::move(manifest));
}

Result<std::vector<std::string>> Manifest::list_backups() const {
    std::vector<std::string> ids;

    std::error_code ec;
    if (!std::filesystem::exists(manifest_dir_, ec)) {
        return Result<std::vector<std::string>>(ids);
    }

    for (const auto& entry : std::filesystem::directory_iterator(manifest_dir_, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            ids.push_back(entry.path().stem().string());
        }
    }

    std::sort(ids.begin(), ids.end());

    return Result<std::vector<std::string>>(std::move(ids));
}

std::filesystem::path Manifest::manifest_path(const std::string& backup_id) const {
    return manifest_dir_ / (backup_id + ".json");
}

} // namespace backups
