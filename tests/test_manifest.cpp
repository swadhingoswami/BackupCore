#include <gtest/gtest.h>
#include "manifest/manifest.h"
#include <fstream>

namespace backups {

class ManifestTest : public ::testing::Test {
protected:
    std::filesystem::path manifest_dir_;

    void SetUp() override {
        manifest_dir_ = std::filesystem::temp_directory_path() / "manifest_test";
        std::filesystem::remove_all(manifest_dir_);
        std::filesystem::create_directories(manifest_dir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(manifest_dir_);
    }
};

TEST_F(ManifestTest, GenerateBackupId) {
    auto id1 = Manifest::generate_backup_id();
    auto id2 = Manifest::generate_backup_id();
    EXPECT_NE(id1, id2);
    EXPECT_TRUE(id1.find("backup_") == 0);
}

TEST_F(ManifestTest, SaveAndLoad) {
    Manifest manifest(manifest_dir_);

    BackupManifest bm;
    bm.backup_id = "test_backup_001";
    bm.timestamp = std::chrono::system_clock::now();

    FileManifest fm;
    fm.path = "dir/file.txt";
    fm.original_size = 100;

    ChunkRecord cr;
    cr.id = "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890";
    cr.size = 50;
    cr.offset = 0;
    fm.chunks.push_back(cr);

    bm.files.push_back(fm);

    ASSERT_TRUE(manifest.save(bm).has_value());

    auto loaded_result = manifest.load("test_backup_001");
    ASSERT_TRUE(loaded_result.has_value());

    const auto& loaded = loaded_result.value();
    EXPECT_EQ(loaded.backup_id, "test_backup_001");
    ASSERT_EQ(loaded.files.size(), 1);
    EXPECT_EQ(loaded.files[0].path.generic_string(), "dir/file.txt");
    EXPECT_EQ(loaded.files[0].original_size, 100);
    ASSERT_EQ(loaded.files[0].chunks.size(), 1);
    EXPECT_EQ(loaded.files[0].chunks[0].id,
              "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
    EXPECT_EQ(loaded.files[0].chunks[0].size, 50);
    EXPECT_EQ(loaded.files[0].chunks[0].offset, 0);
}

TEST_F(ManifestTest, SaveMultipleFiles) {
    Manifest manifest(manifest_dir_);

    BackupManifest bm;
    bm.backup_id = "multi_file_test";
    bm.timestamp = std::chrono::system_clock::now();

    for (int i = 0; i < 3; i++) {
        FileManifest fm;
        fm.path = "file" + std::to_string(i) + ".txt";
        fm.original_size = 100 + i;

        ChunkRecord cr;
        cr.id = "chunk_" + std::to_string(i);
        cr.size = 50;
        cr.offset = 0;
        fm.chunks.push_back(cr);

        bm.files.push_back(fm);
    }

    ASSERT_TRUE(manifest.save(bm).has_value());

    auto result = manifest.load("multi_file_test");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().files.size(), 3);
}

TEST_F(ManifestTest, LoadNonexistent) {
    Manifest manifest(manifest_dir_);
    auto result = manifest.load("nonexistent");
    EXPECT_TRUE(result.has_error());
}

TEST_F(ManifestTest, ListBackups) {
    Manifest manifest(manifest_dir_);

    BackupManifest bm1;
    bm1.backup_id = "backup_alpha";
    bm1.timestamp = std::chrono::system_clock::now();
    ASSERT_TRUE(manifest.save(bm1).has_value());

    BackupManifest bm2;
    bm2.backup_id = "backup_beta";
    bm2.timestamp = std::chrono::system_clock::now();
    ASSERT_TRUE(manifest.save(bm2).has_value());

    auto list_result = manifest.list_backups();
    ASSERT_TRUE(list_result.has_value());
    EXPECT_EQ(list_result.value().size(), 2);
}

TEST_F(ManifestTest, EmptyList) {
    Manifest manifest(manifest_dir_);
    auto result = manifest.list_backups();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}

} // namespace backups
