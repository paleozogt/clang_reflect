#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>
#include <vector>
#include <string>

#include "clang-c/Index.h"

#define SEARCH_START "#include <...> search starts here:"
#define SEARCH_END   "End of search list."

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

inline std::string getString(CXString cxstring) {
    std::string str = clang_getCString(cxstring);
    clang_disposeString(cxstring);
    return str;
}

std::vector<std::string> getSystemIncludePaths() {
    char input[PATH_MAX] = "";
    char output[PATH_MAX] = "";
    tmpnam(output);
    tmpnam(input);

    std::ofstream dummy(input, std::ios_base::out|std::ios_base::binary);
    dummy << std::endl;
    dummy.close();

    char cmd[PATH_MAX*4] = "";
    std::snprintf(cmd, sizeof(cmd), "g++ -E -x c++ -v %s > %s 2> %s", input, output, output);
    int exitCode = std::system(cmd);

    std::vector<std::string> paths;
    std::string line;
    std::ifstream stream(output, std::ios_base::in|std::ios_base::binary);
    while (stream && line != SEARCH_START) {
        std::getline(stream, line);
    }
    std::getline(stream, line);
    while (stream && line != SEARCH_END) {
        ltrim(line);
        std::getline(stream, line);
        paths.push_back(line);
    }

    return paths;
}

std::vector<std::string> getClangArgs() {
    std::vector<std::string> args = { "-std=c++11" };
    for (const auto &path : getSystemIncludePaths()) {
        args.push_back("-I" + path);
    }
    return args;
}

int main(int argc, const char *argv[]) {
    if (argc <= 1) {
        std::cout << argv[0] << " " << "[INPUT]" << std::endl;
        return 0;
    }

    std::string input = argv[1];

    // create clang arguments
    auto clangArgsVec = getClangArgs();
    const char *clangArgs[clangArgsVec.size()];
    for (size_t idx = 0; idx < clangArgsVec.size(); idx++) {
        clangArgs[idx] = clangArgsVec[idx].data();
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit tu = clang_parseTranslationUnit(index, input.data(),
                                                      clangArgs, clangArgsVec.size(),
                                                      nullptr, 0, 0);

    std::cout << getString(clang_getTranslationUnitSpelling(tu)) << std::endl;

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
