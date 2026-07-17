#include <gtest/gtest.h>
#include "engine/backup_engine.h"
#include "engine/restore_engine.h"
#include "common/config.h"
#include <fstream>
#include <cstdio>

namespace backups {

class IntegrationTest : public ::testing::Test {
protected:
    std::filesystem::path source_dir_;
    std::filesystem::path restore_dir_;
    std::filesystem::path repo_dir_;
    Config config_;

    void SetUp() override {
        auto tmp = std::filesystem::temp_directory_path() / "backupcore_integration";
        std::filesystem::remove_all(tmp);

        source_dir_ = tmp / "source";
        restore_dir_ = tmp / "restore";
        repo_dir_ = tmp / "repo";

        std::filesystem::create_directories(source_dir_);
        std::filesystem::create_directories(restore_dir_);

        std::ofstream(source_dir_ / "readme.txt") << "BackupCore integration test\n";
        std::ofstream(source_dir_ / "data.bin") << std::string(1000, 'Z');

        std::filesystem::create_directories(source_dir_ / "docs");
        std::ofstream(source_dir_ / "docs" / "notes.md") << "# Notes\n\nTest content.\n";

        config_.repository_path = repo_dir_;
        config_.worker_threads = 4;
        config_.chunk_size = 4 * 1024 * 1024;
    }

    void TearDown() override {
        std::filesystem::remove_all(
            std::filesystem::temp_directory_path() / "backupcore_integration");
    }
};

TEST_F(IntegrationTest, BackupAndRestore) {
    BackupEngine backup(config_);
    auto backup_result = backup.run_backup(source_dir_);
    ASSERT_TRUE(backup_result.has_value()) << backup_result.error().message;

    std::string backup_id = backup_result.value();

    RestoreEngine restore(config_);
    auto restore_result = restore.run_restore(backup_id, restore_dir_);
    ASSERT_TRUE(restore_result.has_value()) << restore_result.error().message;

    EXPECT_TRUE(std::filesystem::exists(restore_dir_ / "readme.txt"));
    EXPECT_TRUE(std::filesystem::exists(restore_dir_ / "data.bin"));
    EXPECT_TRUE(std::filesystem::exists(restore_dir_ / "docs" / "notes.md"));

    std::ifstream orig(source_dir_ / "readme.txt");
    std::ifstream rest(restore_dir_ / "readme.txt");
    std::string orig_content((std::istreambuf_iterator<char>(orig)),
                              std::istreambuf_iterator<char>());
    std::string rest_content((std::istreambuf_iterator<char>(rest)),
                              std::istreambuf_iterator<char>());
    EXPECT_EQ(orig_content, rest_content);
}

TEST_F(IntegrationTest, ListBackupsAfterBackup) {
    BackupEngine backup(config_);
    auto backup_result = backup.run_backup(source_dir_);
    ASSERT_TRUE(backup_result.has_value());

    RestoreEngine restore(config_);
    auto list_result = restore.list_backups();
    ASSERT_TRUE(list_result.has_value());
}

TEST_F(IntegrationTest, RestoreInvalidBackup) {
    RestoreEngine restore(config_);
    auto result = restore.run_restore("nonexistent_backup_id", restore_dir_);
    EXPECT_TRUE(result.has_error());
}

TEST_F(IntegrationTest, DeduplicatesIdenticalFiles) {
    std::ofstream(source_dir_ / "copy1.txt") << "identical content across files\n";
    std::ofstream(source_dir_ / "copy2.txt") << "identical content across files\n";
    std::ofstream(source_dir_ / "unique.txt") << "unique content here\n";

    BackupEngine backup(config_);
    auto result = backup.run_backup(source_dir_);
    ASSERT_TRUE(result.has_value());

    Repository repo(repo_dir_);
    ASSERT_TRUE(repo.initialize().has_value());

    EXPECT_GT(repo.unique_chunk_count(), 0);
    EXPECT_LT(repo.unique_chunk_count(), 10);
}

} // namespace backups
