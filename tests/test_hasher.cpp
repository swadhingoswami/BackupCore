#include <gtest/gtest.h>
#include "hasher/hasher.h"

namespace backups {

TEST(HasherTest, KnownHash) {
    ChunkData data;
    std::string input = "hello";
    for (char c : input) {
        data.push_back(static_cast<std::byte>(c));
    }

    Hasher hasher;
    auto result = hasher.hash(data);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value(),
              "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(HasherTest, EmptyData) {
    ChunkData data;
    Hasher hasher;
    auto result = hasher.hash(data);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result.value(),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(HasherTest, IdenticalInputSameHash) {
    std::string input = "test data for hashing";
    ChunkData data1, data2;
    for (char c : input) data1.push_back(static_cast<std::byte>(c));
    for (char c : input) data2.push_back(static_cast<std::byte>(c));

    Hasher hasher;
    auto r1 = hasher.hash(data1);
    auto r2 = hasher.hash(data2);
    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r1.value(), r2.value());
}

TEST(HasherTest, DifferentInputDifferentHash) {
    ChunkData data1, data2;
    for (char c : std::string("abc")) data1.push_back(static_cast<std::byte>(c));
    for (char c : std::string("xyz")) data2.push_back(static_cast<std::byte>(c));

    Hasher hasher;
    auto r1 = hasher.hash(data1);
    auto r2 = hasher.hash(data2);
    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());
    EXPECT_NE(r1.value(), r2.value());
}

TEST(HasherTest, HexOutputFormat) {
    ChunkData data;
    for (char c : std::string("test")) data.push_back(static_cast<std::byte>(c));

    Hasher hasher;
    auto result = hasher.hash(data);
    ASSERT_TRUE(result.has_value());

    const auto& hex = result.value();
    EXPECT_EQ(hex.size(), 64);
    for (char c : hex) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

} // namespace backups
