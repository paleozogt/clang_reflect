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

inline std::string basename(const std::string &path) {
    size_t slash_idx= path.find_last_of("\\/");
    size_t size= path.size();
    if (slash_idx == size-1) {
        slash_idx= path.find_last_of("\\/", slash_idx-1);
        size--;
    }
    if (slash_idx != std::string::npos) {
        size_t start_idx= slash_idx + 1;
        return path.substr(start_idx, size-start_idx);
    } else {
        return path.substr(0, size);
    }
}

inline std::string extension(const std::string &path) {
    size_t dot_idx= path.find_last_of(".");
    if (dot_idx != std::string::npos) {
        return path.substr(dot_idx);
    } else {
        return "";
    }
}

inline std::string replace_ext(const std::string &path, const std::string &ext) {
    std::string newPath= path;
    size_t pos= path.find_last_of(".");
    if (pos != std::string::npos) {
        return newPath.replace(pos, std::string::npos, ext);
    } else {
        return newPath;
    }
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

    std::string inputFile = argv[1];
    std::string outputFile = replace_ext(basename(inputFile), "") + "Reflect" + extension(inputFile);

    // create clang arguments
    auto clangArgsVec = getClangArgs();
    const char *clangArgs[clangArgsVec.size()];
    for (size_t idx = 0; idx < clangArgsVec.size(); idx++) {
        clangArgs[idx] = clangArgsVec[idx].data();
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit tu = clang_parseTranslationUnit(index, inputFile.data(),
                                                      clangArgs, clangArgsVec.size(),
                                                      nullptr, 0, 0);

    std::cout << getString(clang_getTranslationUnitSpelling(tu)) << std::endl;

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
