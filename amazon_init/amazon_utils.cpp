#include <cstdlib>
#include <cstring>
#include <string>
#include <limits.h>

#include "include/amazon_utils.h"

const char* resolve_symlink(const std::string& path) {
    static char resolved_path[PATH_MAX];
    if (realpath(path.c_str(), resolved_path)) {
        return resolved_path;
    }
    return nullptr;
}
