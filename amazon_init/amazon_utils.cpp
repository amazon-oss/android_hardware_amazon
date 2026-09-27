#include <cstdlib>
#include <cstring>
#include <string>
#include <limits.h>

#include "include/amazon_utils.h"

std::string parse_mac(const std::string& mac) {
    std::string result;

    result += mac.substr(0, 2) + ':';
    result += mac.substr(2, 2) + ':';
    result += mac.substr(4, 2) + ':';
    result += mac.substr(6, 2) + ':';
    result += mac.substr(8, 2) + ':';
    result += mac.substr(10, 2);

    return result;
}

std::string resolve_symlink(const std::string& path) {
    char resolved_path[PATH_MAX];
    if (realpath(path.c_str(), resolved_path)) {
        return resolved_path;
    }
    return "";
}
