#include "cli.h"
#include "common/config.h"
#include "engine/backup_engine.h"
#include "engine/restore_engine.h"
#include <iostream>
#include <filesystem>

namespace backups {

int CLI::run(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "version" || command == "--version" || command == "-v") {
        print_version();
        return 0;
    }

    if (command == "help" || command == "--help" || command == "-h") {
        print_usage();
        return 0;
    }

    auto config_path = std::filesystem::path("config.json");
    auto config = load_config(config_path);

    if (command == "backup") {
        if (argc < 3) {
            std::cerr << "Usage: backupcore backup <source_dir>\n";
            return 1;
        }

        std::filesystem::path source_dir = argv[2];
        if (!std::filesystem::exists(source_dir)) {
            std::cerr << "Source directory does not exist: " << source_dir << "\n";
            return 1;
        }

        BackupEngine engine(config);
        auto result = engine.run_backup(source_dir);

        if (result.has_error()) {
            std::cerr << "Backup failed: " << result.error() << "\n";
            return 1;
        }

        std::cout << "Backup ID: " << result.value() << "\n";
        return 0;
    }

    if (command == "restore") {
        if (argc < 4) {
            std::cerr << "Usage: backupcore restore <backup_id> <output_dir>\n";
            return 1;
        }

        std::string backup_id = argv[2];
        std::filesystem::path output_dir = argv[3];

        RestoreEngine engine(config);
        auto result = engine.run_restore(backup_id, output_dir);

        if (result.has_error()) {
            std::cerr << "Restore failed: " << result.error() << "\n";
            return 1;
        }

        return 0;
    }

    if (command == "list") {
        RestoreEngine engine(config);
        auto result = engine.list_backups();

        if (result.has_error()) {
            std::cerr << "Failed to list backups: " << result.error() << "\n";
            return 1;
        }

        return 0;
    }

    std::cerr << "Unknown command: " << command << "\n";
    print_usage();
    return 1;
}

void CLI::print_usage() const {
    std::cout << "BackupCore - Educational Backup Engine\n";
    std::cout << "Usage:\n";
    std::cout << "  backupcore backup  <source_dir>   Create a backup\n";
    std::cout << "  backupcore restore <backup_id> <output_dir>  Restore a backup\n";
    std::cout << "  backupcore list                    List available backups\n";
    std::cout << "  backupcore version                 Show version\n";
    std::cout << "  backupcore help                    Show this help\n";
}

void CLI::print_version() const {
    std::cout << "BackupCore version 1.0.0\n";
}

} // namespace backups
