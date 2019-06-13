#include "clang_reflect.hpp"
#include "cli_util.hpp"
#include "macro_util.hpp"

#include <iostream>

#define INCLUDE_PATH_FLAG "-I"
#define HELP_FLAG         "--help"

int main(int argc, const char *argv[]) {
    std::cout << "clang_reflect v" <<  meta_stringify(VERSION) << std::endl;

    auto args = util::parseCli(argc, argv, { INCLUDE_PATH_FLAG, HELP_FLAG });
    auto inputFiles = args[""];
    if (inputFiles.empty() || !args[HELP_FLAG].empty()) {
        std::cout << "Usage: " << argv[0] << " " << "[-I/path/to/dir] [INPUT]" << std::endl;
        return 0;
    }

    clang::ClangReflect generator(args[INCLUDE_PATH_FLAG]);
    for (const auto &inputFile : inputFiles) {
        std::cout << generator.getClangCli(inputFile);
        generator.generate(inputFile);
    }

    return 0;
}
