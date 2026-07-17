#include <gtest/gtest.h>
#include "repository/repository.h"
#include <fstream>

namespace backups {

class RepositoryTest : public ::testing::Test {
protected:
    std::filesystem::path repo_path_;

    void SetUp() override {
        repo_path_ = std::filesystem::temp_directory_path() / "repo_test";
        std::filesystem::remove_all(repo_path_);
    }

    void TearDown() override {
        std::filesystem::remove_all(repo_path_);
    }
};

TEST_F(RepositoryTest, InitializeCreatesDirectories) {
    Repository repo(repo_path_);
    auto result = repo.initialize();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(std::filesystem::exists(repo_path_));
    EXPECT_TRUE(std::filesystem::exists(repo_path_ / "chunks"));
}

TEST_F(RepositoryTest, StoreAndRetrieve) {
    Repository repo(repo_path_);
    ASSERT_TRUE(repo.initialize().has_value());

    ChunkData data(100, std::byte{42});
    ChunkID id = "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890";

    auto store_result = repo.store(id, data);
    ASSERT_TRUE(store_result.has_value());
    EXPECT_TRUE(repo.contains(id));
    EXPECT_EQ(repo.unique_chunk_count(), 1);
    EXPECT_EQ(repo.total_stored_bytes(), 100);

    auto retrieve_result = repo.retrieve(id);
    ASSERT_TRUE(retrieve_result.has_value());
    EXPECT_EQ(retrieve_result.value().size(), 100);
    EXPECT_EQ(retrieve_result.value()[0], std::byte{42});
}

TEST_F(RepositoryTest, Deduplication) {
    Repository repo(repo_path_);
    ASSERT_TRUE(repo.initialize().has_value());

    ChunkData data(50, std::byte{7});
    ChunkID id = "dedup_test_id_1234567890abcdef1234567890abcdef1234567890abcdef";

    ASSERT_TRUE(repo.store(id, data).has_value());
    ASSERT_TRUE(repo.store(id, data).has_value());
    EXPECT_EQ(repo.unique_chunk_count(), 1);
    EXPECT_EQ(repo.total_stored_bytes(), 50);
}

TEST_F(RepositoryTest, RetrieveMissingChunk) {
    Repository repo(repo_path_);
    ASSERT_TRUE(repo.initialize().has_value());

    auto result = repo.retrieve("nonexistent_chunk_id_1234567890abcdef1234567890abcdef");
    ASSERT_TRUE(result.has_error());
}

TEST_F(RepositoryTest, ReinitializationFindsExistingChunks) {
    Repository repo(repo_path_);
    ASSERT_TRUE(repo.initialize().has_value());

    ChunkData data(64, std::byte{1});
    ChunkID id = "reinit_test_id_1234567890abcdef1234567890abcdef1234567890abcdef";
    ASSERT_TRUE(repo.store(id, data).has_value());

    Repository repo2(repo_path_);
    ASSERT_TRUE(repo2.initialize().has_value());
    EXPECT_TRUE(repo2.contains(id));
    EXPECT_EQ(repo2.unique_chunk_count(), 1);
    EXPECT_EQ(repo2.total_stored_bytes(), 64);
}

TEST_F(RepositoryTest, StoresMultipleChunks) {
    Repository repo(repo_path_);
    ASSERT_TRUE(repo.initialize().has_value());

    ChunkData data1(10, std::byte{1});
    ChunkData data2(20, std::byte{2});
    ChunkData data3(30, std::byte{3});

    ChunkID id1 = "id1_1234567890abcdef1234567890abcdef1234567890abcdef1234";
    ChunkID id2 = "id2_1234567890abcdef1234567890abcdef1234567890abcdef1234";
    ChunkID id3 = "id3_1234567890abcdef1234567890abcdef1234567890abcdef1234";

    ASSERT_TRUE(repo.store(id1, data1).has_value());
    ASSERT_TRUE(repo.store(id2, data2).has_value());
    ASSERT_TRUE(repo.store(id3, data3).has_value());

    EXPECT_EQ(repo.unique_chunk_count(), 3);
    EXPECT_EQ(repo.total_stored_bytes(), 60);
}

} // namespace backups
