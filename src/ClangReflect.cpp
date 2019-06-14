#include "ClangReflect.hpp"
#include "CliUtil.hpp"
#include "MacroUtil.hpp"

#include <iostream>

#define HELP_FLAG         "--help"

int main(int argc, const char *argv[]) {
    std::cout << "clang_reflect v" <<  meta_stringify(VERSION) << std::endl;

    auto args = util::parseCli(argc, argv);
    auto inputFiles = args[""];
    if (inputFiles.empty() || !args[HELP_FLAG].empty()) {
        std::cout << "Usage: " << argv[0] << " " << "[-I/path/to/dir] [INPUT]" << std::endl;
        return 0;
    }

    try {
        clang::ClangReflect generator(args["-"]);
        for (const auto &inputFile : inputFiles) {
            std::cout << generator.getClangCli(inputFile);
            generator.generate(inputFile);
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
