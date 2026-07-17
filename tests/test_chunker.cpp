#include <gtest/gtest.h>
#include "chunker/chunker.h"

namespace backups {

TEST(ChunkerTest, SingleChunkSmallData) {
    ChunkData data(100);
    Chunker chunker(4096);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 1);
    EXPECT_EQ(result.value()[0].data.size(), 100);
    EXPECT_EQ(result.value()[0].offset, 0);
}

TEST(ChunkerTest, MultipleChunks) {
    ChunkData data(5000);
    Chunker chunker(4096);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 2);
    EXPECT_EQ(result.value()[0].data.size(), 4096);
    EXPECT_EQ(result.value()[1].data.size(), 5000 - 4096);
    EXPECT_EQ(result.value()[0].offset, 0);
    EXPECT_EQ(result.value()[1].offset, 4096);
}

TEST(ChunkerTest, ExactMultiple) {
    ChunkData data(8192);
    Chunker chunker(4096);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 2);
    EXPECT_EQ(result.value()[0].data.size(), 4096);
    EXPECT_EQ(result.value()[1].data.size(), 4096);
}

TEST(ChunkerTest, EmptyData) {
    ChunkData data;
    Chunker chunker(4096);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());
}

TEST(ChunkerTest, ChunkIndices) {
    ChunkData data(9000);
    Chunker chunker(4096);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value().size(), 3);
    EXPECT_EQ(result.value()[0].index, 0);
    EXPECT_EQ(result.value()[1].index, 1);
    EXPECT_EQ(result.value()[2].index, 2);
}

TEST(ChunkerTest, PreservesData) {
    ChunkData data = {std::byte{1}, std::byte{2}, std::byte{3}};
    Chunker chunker(2);
    auto result = chunker.chunk(data);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value().size(), 2);
    EXPECT_EQ(result.value()[0].data[0], std::byte{1});
    EXPECT_EQ(result.value()[0].data[1], std::byte{2});
    EXPECT_EQ(result.value()[1].data[0], std::byte{3});
}

} // namespace backups
