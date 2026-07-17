#include "config.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <cctype>
#include <stdexcept>
#include <cstdlib>


namespace backups {
namespace {

class JsonParser {
public:
    explicit JsonParser(std::string_view input) : input_(input), pos_(0) {}

    std::unordered_map<std::string, std::variant<std::string, int64_t>> parse_object() {
        skip_whitespace();
        expect('{');

        std::unordered_map<std::string, std::variant<std::string, int64_t>> result;

        skip_whitespace();
        if (peek() == '}') {
            pos_++;
            return result;
        }

        while (true) {
            skip_whitespace();
            auto key = parse_string();
            skip_whitespace();
            expect(':');
            skip_whitespace();

            if (peek() == '"') {
                result[std::move(key)] = parse_string();
            } else {
                result[std::move(key)] = parse_integer();
            }

            skip_whitespace();
            if (peek() == '}') {
                pos_++;
                break;
            }
            expect(',');
        }

        return result;
    }

private:
    std::string_view input_;
    size_t pos_ = 0;

    void skip_whitespace() {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_]))) {
            pos_++;
        }
    }

    char peek() const {
        return pos_ < input_.size() ? input_[pos_] : '\0';
    }

    void expect(char c) {
        skip_whitespace();
        if (pos_ >= input_.size() || input_[pos_] != c) {
            throw std::runtime_error(std::string("Expected '") + c + "' at position " + std::to_string(pos_));
        }
        pos_++;
    }

    std::string parse_string() {
        skip_whitespace();
        if (peek() != '"') {
            throw std::runtime_error("Expected string at position " + std::to_string(pos_));
        }
        pos_++;
        std::string result;
        while (pos_ < input_.size() && input_[pos_] != '"') {
            if (input_[pos_] == '\\') {
                pos_++;
                if (pos_ < input_.size()) {
                    switch (input_[pos_]) {
                        case '"': result += '"'; break;
                        case '\\': result += '\\'; break;
                        case 'n': result += '\n'; break;
                        case 't': result += '\t'; break;
                        default: result += input_[pos_]; break;
                    }
                }
            } else {
                result += input_[pos_];
            }
            pos_++;
        }
        if (pos_ >= input_.size()) {
            throw std::runtime_error("Unterminated string");
        }
        pos_++;
        return result;
    }

    int64_t parse_integer() {
        skip_whitespace();
        size_t start = pos_;
        if (peek() == '-') pos_++;
        while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
            pos_++;
        }
        if (pos_ == start) {
            throw std::runtime_error("Expected number at position " + std::to_string(pos_));
        }
        return std::stoll(std::string(input_.substr(start, pos_ - start)));
    }
};

} // anonymous namespace

Config load_config(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return Config{};
    }
    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();
    if (content.empty()) {
        return Config{};
    }

    JsonParser parser(content);
    auto parsed = parser.parse_object();

    Config config;

    auto it = parsed.find("chunk_size");
    if (it != parsed.end() && std::holds_alternative<int64_t>(it->second)) {
        config.chunk_size = static_cast<size_t>(std::get<int64_t>(it->second));
    }

    it = parsed.find("worker_threads");
    if (it != parsed.end() && std::holds_alternative<int64_t>(it->second)) {
        config.worker_threads = static_cast<int>(std::get<int64_t>(it->second));
    }

    it = parsed.find("repository_path");
    if (it != parsed.end() && std::holds_alternative<std::string>(it->second)) {
        config.repository_path = std::get<std::string>(it->second);
    }

    return config;
}

} // namespace backups
