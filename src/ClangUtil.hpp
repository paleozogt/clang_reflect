#pragma once

#include <fstream>
#include <cstdlib>
#include <climits>

#include <string>
#include <vector>

#include "StringUtil.hpp"
#include "SysUtil.hpp"
#include "clang-c/Index.h"

namespace clang {
    inline std::string getString(CXString cxstring) {
        const char *cstring = clang_getCString(cxstring);
        std::string str = cstring ? cstring : "";
        clang_disposeString(cxstring);
        return str;
    }

    inline std::string getString(CX_CXXAccessSpecifier accessSpecifier) {
        switch (accessSpecifier) {
            case CX_CXXPublic:      return "public";
            case CX_CXXProtected:   return "protected";
            case CX_CXXPrivate:     return "private";
            default:                return "invalid";
        }
    }

    inline std::string getString(CXErrorCode code) {
        switch (code) {
            case CXError_Success:           return "CXError_Success";
            case CXError_Failure:           return "CXError_Failure";
            case CXError_Crashed:           return "CXError_Crashed";
            case CXError_InvalidArguments:  return "CXError_InvalidArguments";
            case CXError_ASTReadError:      return "CXError_ASTReadError";
            default:                        return "CXError";
        }
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

    inline std::vector<std::string> getGccOptions(const std::string &optionsLine) {
        std::vector<std::string> options;

        // note that out of expediency we're assuming that
        // the options themselves don't have spaces
        options = util::split(optionsLine, ' ');
        for (auto &option : options) {
            option = util::replace(option, "'", "");
        }

        return options;
    }

    inline std::vector<std::string> getCompilerFlags() {
        std::vector<std::string> flags, paths;

#if defined(WIN32)
        paths = util::split(util::getEnv("INCLUDE"), ';');
#else
        const static std::string COLLECT_GCC_OPTIONS = "COLLECT_GCC_OPTIONS=";
        const static std::string TARGET = "Target:";
        const static std::string SEARCH_START = "#include <...> search starts here:";
        const static std::string SEARCH_END   = "End of search list.";

        char output[FILENAME_MAX] = "";
        tmpnam(output);

        std::string gpp = util::getEnv("CXX", util::getEnv("CC", "g++"));

        char cmd[FILENAME_MAX*4] = "";
        std::snprintf(cmd, sizeof(cmd), "%s -E -x c++ -v %s > %s 2> %s", gpp.data(), "/dev/null", "/dev/null", output);
        int exitCode = std::system(cmd);

        std::string line;
        std::ifstream stream(output, std::ios_base::in|std::ios_base::binary);
        while (stream && line != SEARCH_START) {
            if (line.find(COLLECT_GCC_OPTIONS) == 0) {
                for (const auto &flag : getGccOptions(util::replace(line, COLLECT_GCC_OPTIONS, ""))) {
                    if (flag.find("-march") == 0 || flag.find("-mfloat-abi") == 0) {
                        flags.push_back(flag);
                    }
                }
            } else if (line.find(TARGET) == 0) {
                line = util::replace(line, TARGET, "");
                util::ltrim(line, ::isspace);
                flags.push_back("--target=" + line);
            }
            std::getline(stream, line);
        }
        std::getline(stream, line);
        while (stream && line != SEARCH_END) {
            util::ltrim(line, ::isspace);
            paths.push_back(line);
            std::getline(stream, line);
        }

        flags.push_back("-nostdinc");
        flags.push_back("-nostdinc++");
#endif

        for (const auto &path : paths) {
            if (!path.empty()) {
                flags.push_back("-I" + path);
            }
        }

        return flags;
    }

    inline std::vector<std::string> getClangArgs(const std::vector<std::string> &clangArgs) {
        std::vector<std::string> args = getCompilerFlags();
        for (const auto &arg : clangArgs) {
            args.push_back(arg);
        }
        return args;
    }

}
