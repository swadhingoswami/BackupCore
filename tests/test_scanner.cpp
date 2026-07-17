#include <gtest/gtest.h>
#include "scanner/scanner.h"
#include <fstream>

namespace backups {

TEST(ScannerTest, ScansFiles) {
    auto tmp = std::filesystem::temp_directory_path() / "scanner_test";
    std::filesystem::remove_all(tmp);
    std::filesystem::create_directories(tmp);

    std::ofstream(tmp / "a.txt") << "hello";
    std::filesystem::create_directories(tmp / "sub");
    std::ofstream(tmp / "sub" / "b.txt") << "world";

    Scanner scanner(tmp);
    auto result = scanner.scan();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().size(), 2);

    std::filesystem::remove_all(tmp);
}

TEST(ScannerTest, EmptyDirectory) {
    auto tmp = std::filesystem::temp_directory_path() / "scanner_empty";
    std::filesystem::remove_all(tmp);
    std::filesystem::create_directories(tmp);

    Scanner scanner(tmp);
    auto result = scanner.scan();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());

    std::filesystem::remove_all(tmp);
}

TEST(ScannerTest, MissingDirectory) {
    Scanner scanner("/nonexistent/path/xyz789");
    auto result = scanner.scan();
    ASSERT_TRUE(result.has_error());
}

TEST(ScannerTest, ReportsFileSizes) {
    auto tmp = std::filesystem::temp_directory_path() / "scanner_sizes";
    std::filesystem::remove_all(tmp);
    std::filesystem::create_directories(tmp);

    std::ofstream(tmp / "data.bin") << std::string(1000, 'X');

    Scanner scanner(tmp);
    auto result = scanner.scan();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value().size(), 1);
    EXPECT_EQ(result.value()[0].size, 1000);

    std::filesystem::remove_all(tmp);
}

} // namespace backups
