#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <set>

#include "clang-c/Index.h"

#define SEARCH_START "#include <...> search starts here:"
#define SEARCH_END   "End of search list."

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
        ltrim(line, ::isspace);
        paths.push_back(line);
        std::getline(stream, line);
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

std::string getDisplayName(std::string name) {
    ltrim(name, ::ispunct);
    rtrim(name, ::ispunct);
    return name;
}

std::string indent(size_t indent) {
    return std::string(indent*4, ' ');
}

void generateReflector(std::ostream &stream, CXCursor cursor) {
    auto namespaces = getNamespaces(cursor);
    size_t nsindent = namespaces.size();

    stream << "#include \""
           << getString(clang_getTranslationUnitSpelling(clang_Cursor_getTranslationUnit(cursor)))
           << "\"" << std::endl
           << std::endl;

    for (size_t idx = 0; idx < namespaces.size(); idx++) {
        stream << indent(idx) << "namespace " << namespaces[idx] << " {" << std::endl;
    }
    stream << std::endl;

    std::vector<std::pair<std::string,std::string>> constsVec = { {"const ", ""}, {"", "const "} };
    for (const auto &consts : constsVec) {
        stream << indent(nsindent) << "template<typename F>"
               << std::endl
               << indent(nsindent) << "void reflect("
               << consts.first << getString(clang_getCursorSpelling(cursor))
               << " &obj"
               << ", "
               << consts.second << "F &f"
               << ") {"
               << std::endl;

        auto fields = getChildrenOfKind(cursor, CXCursor_FieldDecl);
        for (const auto &field : fields) {
            auto spelling = getString(clang_getCursorSpelling(field));
            stream << indent(nsindent+1)
                   << "f("
                   << "\"" << getDisplayName(spelling) << "\""
                   << ", "
                   << "\"" << getString(clang_getTypeSpelling(clang_getCursorType(field))) << "\""
                    << ", "
                   << "obj." << spelling
                   << ");"
                   << std::endl;
        }

        stream << indent(nsindent)
               << "}"
               << std::endl
               << std::endl;
    }

    for (size_t idx = 0; idx < namespaces.size(); idx++) {
        stream << indent(nsindent-idx-1) << "}" << std::endl;
    }
}

int main(int argc, const char *argv[]) {
    if (argc <= 1) {
        std::cout << argv[0] << " " << "[INPUT]" << std::endl;
        return 0;
    }

    std::string inputFile = argv[1];
    std::string outputFile = replace_ext(basename(inputFile), "") + "Reflect" + extension(inputFile);
    std::ofstream stream(outputFile, std::ios_base::out|std::ios_base::binary);

    // create clang arguments
    auto clangArgsVec = getClangArgs();
    const char *clangArgs[clangArgsVec.size()];
    for (size_t idx = 0; idx < clangArgsVec.size(); idx++) {
        clangArgs[idx] = clangArgsVec[idx].data();
    }

    CXIndex index = clang_createIndex(0, 0);
    for (const auto &arg : clangArgs) {
        std::cout << arg << " ";
    }
    std::cout << std::endl;
    CXTranslationUnit tu = clang_parseTranslationUnit(index, inputFile.data(),
                                                      clangArgs, clangArgsVec.size(),
                                                      nullptr, 0, 0);

    auto classes = getChildrenOfKind(clang_getTranslationUnitCursor(tu), CXCursor_ClassDecl);
    for (const auto &clazz : classes) {
        if (clang_Location_isFromMainFile(clang_getCursorLocation(clazz)) &&
            !getChildrenOfKind(clazz, CXCursor_FunctionTemplate, "reflect").empty()) {
            generateReflector(stream, clazz);
        }
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
