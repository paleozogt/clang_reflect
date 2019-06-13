#include <iostream>
#include <fstream>
#include <cstdio>
#include <climits>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "clang-c/Index.h"

#define SEARCH_START "#include <...> search starts here:"
#define SEARCH_END   "End of search list."

#define INCLUDE_PATH_FLAG "-I"

template<typename F>
inline void ltrim(std::string &s, F &f) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](int ch) {
        return !f(ch);
    }));
}

template<typename F>
inline void rtrim(std::string &s, F &f) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](int ch) {
        return !f(ch);
    }).base(), s.end());
}

inline std::string getString(CXString cxstring) {
    std::string str = clang_getCString(cxstring);
    clang_disposeString(cxstring);
    return str;
}

std::vector<std::string> getSystemIncludePaths() {
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
        ltrim(line, ::isspace);
        paths.push_back(line);
        std::getline(stream, line);
    }

    return paths;
}

std::vector<std::string> getClangArgs(const std::vector<std::string> &includePaths) {
    std::vector<std::string> args = { "-std=c++11", "-E" };
    for (const auto &path : getSystemIncludePaths()) {
        args.push_back(INCLUDE_PATH_FLAG + path);
    }
    for (const auto &path : includePaths) {
        args.push_back(INCLUDE_PATH_FLAG + path);
    }
    return args;
}

std::string getReflectHeaderName(CXCursor clazz) {
    return getString(clang_getCursorSpelling(clazz)) + "Reflect.hpp";
}

template<typename F>
void visitChildren(CXCursor parent, const F &f) {
    clang_visitChildren(parent, [](CXCursor cursor, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
        F &f = *static_cast<F*>(client_data);
        return f(cursor);
    }, (CXClientData)&f);
}

std::vector<std::string> getNamespaces(CXCursor cursor) {
    std::vector<std::string> namespaces;

    auto parent = clang_getCursorSemanticParent(cursor);
    while (clang_getCursorKind(parent) != CXCursor_TranslationUnit) {
        namespaces.push_back(getString(clang_getCursorSpelling(parent)));
        parent = clang_getCursorLexicalParent(parent);
    }

    return std::vector<std::string>(namespaces.rbegin(), namespaces.rend());
}

std::vector<CXCursor> getChildrenOfKind(CXCursor parent, CXCursorKind kind, const std::string &name = "") {
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

bool isReflectable(CXCursor clazz) {
    auto children = getChildrenOfKind(clazz, CXCursor_FunctionTemplate, "reflect");
    return !children.empty();
}

std::string indent(size_t indent) {
    return std::string(indent*4, ' ');
}

void generateReflector(std::ostream &stream, CXCursor cursor) {
    auto bases = getChildrenOfKind(cursor, CXCursor_CXXBaseSpecifier);
    auto namespaces = getNamespaces(cursor);
    size_t nsindent = namespaces.size();
    const std::string reflectMethod = "reflect";

    // include the class definiton
    stream << "#include \""
           << getString(clang_getTranslationUnitSpelling(clang_Cursor_getTranslationUnit(cursor)))
           << "\"" << std::endl
           << std::endl;

    // include generated base class headers
    for (const auto &base : bases) {
        CXCursor clazz = clang_getTypeDeclaration(clang_getCursorType(base));
        if (isReflectable(clazz)) {
            stream << "#include \""
                   << getReflectHeaderName(clazz)
                   << "\"" << std::endl
                   << std::endl;
        }
    }

    // namespaces
    for (size_t idx = 0; idx < namespaces.size(); idx++) {
        stream << indent(idx) << "namespace " << namespaces[idx] << " {" << std::endl;
    }
    stream << std::endl;

    std::vector<std::pair<std::string,std::string>> constsVec = { {"const ", ""}, {"", "const "}, { "const ", "const " }, { "", "" } };
    for (const auto &consts : constsVec) {
        // function definition
        stream << indent(nsindent) << "template<typename F>"
               << std::endl
               << indent(nsindent) << "void " << reflectMethod << "("
               << consts.first << getString(clang_getCursorSpelling(cursor))
               << " &obj"
               << ", "
               << consts.second << "F &f"
               << ") {"
               << std::endl;

        // reflect on base-classes
        for (const auto &base : bases) {
            stream << indent(nsindent+1)
                   << reflectMethod << "("
                   << "dynamic_cast<" << consts.first << getString(clang_getTypeSpelling(clang_getCursorType(base))) << "&>(obj)"
                   << ", "
                   << "f"
                   << ");"
                   << std::endl;
        }

        // reflect on each field
        auto fields = getChildrenOfKind(cursor, CXCursor_FieldDecl);
        for (const auto &field : fields) {
            auto spelling = getString(clang_getCursorSpelling(field));
            stream << indent(nsindent+1)
                   << "f("
                   << "\"" << spelling << "\""
                   << ", "
                   << "\"" << getString(clang_getTypeSpelling(clang_getCursorType(field))) << "\""
                    << ", "
                   << "obj." << spelling
                   << ");"
                   << std::endl;
        }

        // close out the function
        stream << indent(nsindent)
               << "}"
               << std::endl
               << std::endl;
    }

    // close out the namespaces
    for (size_t idx = 0; idx < namespaces.size(); idx++) {
        stream << indent(nsindent-idx-1) << "}" << std::endl;
    }
}

std::map<std::string, std::vector<std::string>> parseCli(int argc, const char *argv[], const std::vector<std::string> &flags) {
    std::map<std::string, std::vector<std::string>> args;
    for (size_t idx = 1; idx < argc; idx++) {
        std::string arg = argv[idx];
        bool found = false;
        for (const auto &flag : flags) {
            if (arg.find(flag, 0) == 0) {
                args[flag].push_back(arg.substr(flag.size()));
                found = true;
                break;
            }
        }
        if (!found) args[""].push_back(arg);
    }

    return args;
}

int main(int argc, const char *argv[]) {
    auto args = parseCli(argc, argv, { INCLUDE_PATH_FLAG });
    if (argc <= 2) {
        std::cout << argv[0] << " " << "[INPUT]" << std::endl;
        return 0;
    }

    std::string inputFile = args[""][0];

    // create clang arguments
    const auto clangArgsVec = getClangArgs(args[INCLUDE_PATH_FLAG]);
    std::vector<const char*> clangArgs(clangArgsVec.size());
    for (size_t idx = 0; idx < clangArgsVec.size(); idx++) {
        clangArgs[idx] = clangArgsVec[idx].data();
    }

    CXIndex index = clang_createIndex(0, 0);
    for (const auto &arg : clangArgs) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
    CXTranslationUnit tu = clang_parseTranslationUnit(index, inputFile.data(),
                                                      clangArgs.data(), clangArgsVec.size(),
                                                      nullptr, 0,
                                                      CXTranslationUnit_DetailedPreprocessingRecord);

    auto classes = getChildrenOfKind(clang_getTranslationUnitCursor(tu), CXCursor_ClassDecl);
    for (const auto &clazz : classes) {
        if (isReflectable(clazz) && clang_Location_isFromMainFile(clang_getCursorLocation(clazz))) {
            std::ofstream stream(getReflectHeaderName(clazz), std::ios_base::out|std::ios_base::binary);
            generateReflector(stream, clazz);
        }
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
