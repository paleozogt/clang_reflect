#pragma once

#include <string>
#include <vector>
#include <map>

namespace util {
    inline std::map<std::string, std::vector<std::string>> parseCli(int argc, const char *argv[]) {
        std::map<std::string, std::vector<std::string>> args;

        for (size_t idx = 1; idx < argc; idx++) {
            std::string arg = argv[idx];

            if (arg.find("--", 0) == 0) {
                args[arg].push_back(argv[idx++]);
            } else if (arg.find('-', 0) == 0) {
                args["-"].push_back(arg);
            } else {
                args[""].push_back(arg);
            }
        }

        return args;
    }
}
