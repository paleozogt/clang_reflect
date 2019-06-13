#pragma once

#include "clang_util.hpp"
#include "file_util.hpp"

#include <sstream>
#include <stdexcept>

namespace clang {
    class ClangReflect {
    public:
        ClangReflect(const std::vector<std::string> &includePaths)
            : clangArgs(getClangArgs(includePaths))
        {
        }

        std::string getReflectHeaderName(CXCursor clazz) const {
            return getString(clang_getCursorSpelling(clazz)) + "Reflect.hpp";
        }

        bool isReflectable(CXCursor clazz) const {
            auto children = getChildrenOfKind(clazz, CXCursor_FunctionTemplate, "reflect");
            return !children.empty();
        }

        std::string getClangCli(const std::string &inputFile) const {
            std::ostringstream stream;

            // log the flags we're using
            stream << "clang++ ";
            for (const auto &arg : clangArgs) {
                stream << "\"" << arg << "\" ";
            }
            stream << "\"" << inputFile << "\"" << std::endl;

            return stream.str();
        }

        void generate(const std::string &inputFile) {
            if (!util::doesFileExist(inputFile)) {
                throw std::invalid_argument("Unable to open " + inputFile);
            }

            // create clang arguments
            std::vector<const char*> clangArgs(this->clangArgs.size());
            for (size_t idx = 0; idx < this->clangArgs.size(); idx++) {
                clangArgs[idx] = this->clangArgs[idx].data();
            }

            CXIndex index = clang_createIndex(0, 0);
            CXTranslationUnit tu = clang_parseTranslationUnit(index, inputFile.data(),
                                                              clangArgs.data(), clangArgs.size(),
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
                stream << util::indent(idx) << "namespace " << namespaces[idx] << " {" << std::endl;
            }
            stream << std::endl;

            std::vector<std::pair<std::string,std::string>> constsVec = { {"const ", ""}, {"", "const "}, { "const ", "const " }, { "", "" } };
            for (const auto &consts : constsVec) {
                // function definition
                stream << util::indent(nsindent) << "template<typename F>"
                       << std::endl
                       << util::indent(nsindent) << "void " << reflectMethod << "("
                       << consts.first << getString(clang_getCursorSpelling(cursor))
                       << " &obj"
                       << ", "
                       << consts.second << "F &f"
                       << ") {"
                       << std::endl;

                // reflect on base-classes
                for (const auto &base : bases) {
                    stream << util::indent(nsindent+1)
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
                    stream << util::indent(nsindent+1)
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
                stream << util::indent(nsindent)
                       << "}"
                       << std::endl
                       << std::endl;
            }

            // close out the namespaces
            for (size_t idx = 0; idx < namespaces.size(); idx++) {
                stream << util::indent(nsindent-idx-1) << "}" << std::endl;
            }
        }

    protected:
        std::vector<std::string> clangArgs;
    };
}
