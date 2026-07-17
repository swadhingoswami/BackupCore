#pragma once

#include <string>
#include <vector>

namespace backups {

class CLI {
public:
    CLI() = default;

    int run(int argc, char* argv[]);

private:
    void print_usage() const;
    void print_version() const;
};

} // namespace backups
