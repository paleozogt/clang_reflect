#pragma once

#include <fstream>
#include <cstdlib>
#include <climits>

#include <string>
#include <vector>

#include "string_util.hpp"
#include "clang-c/Index.h"

namespace clang {
    inline std::string getString(CXString cxstring) {
        std::string str = clang_getCString(cxstring);
        clang_disposeString(cxstring);
        return str;
    }

    template<typename F>
    inline void visitChildren(CXCursor parent, const F &f) {
        clang_visitChildren(parent, [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
            F &f = *static_cast<F*>(client_data);
            return f(cursor);
        }, (CXClientData)&f);
    }

    inline std::vector<std::string> getNamespaces(CXCursor cursor) {
        std::vector<std::string> namespaces;

        auto parent = clang_getCursorSemanticParent(cursor);
        while (clang_getCursorKind(parent) != CXCursor_TranslationUnit) {
            namespaces.push_back(getString(clang_getCursorSpelling(parent)));
            parent = clang_getCursorLexicalParent(parent);
        }

        return std::vector<std::string>(namespaces.rbegin(), namespaces.rend());
    }

    inline std::vector<CXCursor> getChildrenOfKind(CXCursor parent, CXCursorKind kind, const std::string &name = "") {
        std::vector<CXCursor> classes;

        visitChildren(parent, [&](CXCursor cursor) {
            if (clang_getCursorKind(cursor) == kind &&
                (name.empty() || name == getString(clang_getCursorSpelling(cursor))))
            {
                classes.push_back(cursor);
            }
            return CXChildVisit_Recurse;
        });

        return classes;
    }

    inline std::vector<std::string> getSystemIncludePaths() {
        const static std::string SEARCH_START = "#include <...> search starts here:";
        const static std::string SEARCH_END   = "End of search list.";

        char input[FILENAME_MAX] = "";
        char output[FILENAME_MAX] = "";
        tmpnam(output);
        tmpnam(input);

        std::ofstream dummy(input, std::ios_base::out|std::ios_base::binary);
        dummy << std::endl;
        dummy.close();

        char cmd[FILENAME_MAX*4] = "";
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
            util::ltrim(line, ::isspace);
            paths.push_back(line);
            std::getline(stream, line);
        }

        return paths;
    }

    inline std::vector<std::string> getClangArgs(const std::vector<std::string> &includePaths) {
        std::vector<std::string> args = { "-std=c++11" };
        for (const auto &path : getSystemIncludePaths()) {
            args.push_back("-I" + path);
        }
        for (const auto &path : includePaths) {
            args.push_back("-I" + path);
        }
        return args;
    }


}
