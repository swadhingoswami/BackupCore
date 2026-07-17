#pragma once

#include <variant>
#include <string>
#include <utility>
#include <stdexcept>
#include <ostream>

namespace backups {

struct Error {
    std::string message;
};

inline std::ostream& operator<<(std::ostream& os, const Error& err) {
    os << err.message;
    return os;
}

template <typename T>
class [[nodiscard]] Result {
public:
    explicit(false) Result(T value) : data_(std::move(value)) {}
    explicit(false) Result(Error error) : data_(std::move(error)) {}

    bool has_value() const { return std::holds_alternative<T>(data_); }
    bool has_error() const { return !has_value(); }

    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }

    Error& error() {
        if (has_error()) return std::get<Error>(data_);
        throw std::logic_error("Result has no error");
    }
    const Error& error() const {
        if (has_error()) return std::get<Error>(data_);
        throw std::logic_error("Result has no error");
    }

    explicit operator bool() const { return has_value(); }

private:
    std::variant<T, Error> data_;
};

template <>
class [[nodiscard]] Result<void> {
public:
    Result() : data_(std::monostate{}) {}
    explicit(false) Result(Error error) : data_(std::move(error)) {}

    bool has_value() const { return std::holds_alternative<std::monostate>(data_); }
    bool has_error() const { return !has_value(); }

    void value() const {}

    Error& error() {
        if (has_error()) return std::get<Error>(data_);
        throw std::logic_error("Result has no error");
    }
    const Error& error() const {
        if (has_error()) return std::get<Error>(data_);
        throw std::logic_error("Result has no error");
    }

    explicit operator bool() const { return has_value(); }

private:
    std::variant<std::monostate, Error> data_;
};

} // namespace backups
