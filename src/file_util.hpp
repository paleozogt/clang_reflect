#pragma once

#include <fstream>

namespace util {
    inline bool doesFileExist(const std::string &file) {
        std::ifstream inputStream(file, std::ios_base::in|std::ios_base::binary);
        return inputStream.good();
    }
}
