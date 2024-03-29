#pragma once

#include "ClangUtil.hpp"
#include "SysUtil.hpp"

#include <sstream>
#include <stdexcept>
#include <iostream>

#define diag_err_builtin_definition_msg "definition of builtin function"

namespace clang {
    class ClangReflect {
    public:
        ClangReflect(const std::vector<std::string> &clangArgs)
            : clangArgs(getClangArgs(clangArgs))
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

            CXIndex index = clang_createIndex(1, 0);
            const unsigned tuOptions = CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_SkipFunctionBodies | CXTranslationUnit_KeepGoing;
            CXTranslationUnit tu;
            CXErrorCode err = clang_parseTranslationUnit2(index, inputFile.data(),
                                                          clangArgs.data(), clangArgs.size(),
                                                          nullptr, 0,
                                                          tuOptions, &tu);

            const unsigned diagnosticOptions = CXDiagnostic_DisplaySourceLocation |
                                               CXDiagnostic_DisplayColumn |
                                               CXDiagnostic_DisplayOption |
                                               CXDiagnostic_DisplaySourceRanges |
                                               CXDiagnostic_DisplayCategoryId |
                                               CXDiagnostic_DisplayCategoryName;

            unsigned numDiagnostics = clang_getNumDiagnostics(tu);
            if (numDiagnostics > 0) {
                auto diagnostic = clang_getDiagnostic(tu, 0);
                auto severity = clang_getDiagnosticSeverity(diagnostic);
                std::string formattedDiag = getString(clang_formatDiagnostic(diagnostic, diagnosticOptions));
                if (severity > CXDiagnostic_Warning && formattedDiag.find(diag_err_builtin_definition_msg) == std::string::npos) {
                    throw std::runtime_error(formattedDiag);
                }
            }

            if (err != CXError_Success) {
                throw std::runtime_error(getString(err));
            }

            auto classes = getChildrenOfKind(clang_getTranslationUnitCursor(tu), CXCursor_ClassDecl);
            for (const auto &clazz : classes) {
                if (isReflectable(clazz) && clang_Location_isFromMainFile(clang_getCursorLocation(clazz))) {
                    std::ofstream stream(getReflectHeaderName(clazz), std::ios_base::out|std::ios_base::binary);

                    generateReflectorPreamble(stream, clazz);
                    generateReflector(stream, clazz, 0, 1);
                    generateReflector(stream, clazz, 0, 2);
                }
            }

            clang_disposeTranslationUnit(tu);
            clang_disposeIndex(index);
        }

        void generateNamespaces(std::ostream &stream, const std::vector<std::string> &names) {
            for (size_t idx = 0; idx < names.size(); idx++) {
                stream << util::indent(idx)
                       << "namespace "
                       << names[idx] << "{"
                       << std::endl
                       << std::endl;
            }
        }

        void generateNamespaceClosures(std::ostream &stream, const std::vector<std::string> &names) {
            for (size_t idx = 0; idx < names.size(); idx++) {
                stream << util::indent(names.size()-idx-1)
                       << "}"
                       << std::endl;
            }
        }

        std::string getFqMacroName(const std::string &fqname) {
            return util::transform(
                util::split(util::replace(fqname, "::", "_"), '_').back(),
                ::toupper
            ) + "_FQNAME";
        }

        void generateReflectorPreamble(std::ostream &stream, CXCursor cursor) {
            // header pragma
            stream << "#pragma once"
                   << std::endl
                   << std::endl;

            // include the class definiton
            stream << "#include \""
                   << getString(clang_getTranslationUnitSpelling(clang_Cursor_getTranslationUnit(cursor)))
                   << "\"" << std::endl
                   << std::endl;

            // include generated base class headers
            for (const auto &base : getChildrenOfKind(cursor, CXCursor_CXXBaseSpecifier)) {
                CXCursor clazz = clang_getTypeDeclaration(clang_getCursorType(base));
                if (isReflectable(clazz)) {
                    stream << "#include \""
                           << getReflectHeaderName(clazz)
                           << "\"" << std::endl;
                }
            }

            // include generated field headers
            for (const auto &field : getChildrenOfKind(cursor, CXCursor_FieldDecl)) {
                CXCursor clazz = clang_getTypeDeclaration(clang_getCursorType(field));
                if (isReflectable(clazz)) {
                    stream << "#include \""
                           << getReflectHeaderName(clazz)
                           << "\"" << std::endl;
                }
            }

            stream << std::endl;

            // type traits
            stream << "#include <type_traits>" 
                   << std::endl
                   << std::endl;

            // macro for fully-qualified class name
            const std::string fqname = getString(clang_getTypeSpelling(clang_getCursorType(cursor)));
            stream << "#define "
                   << getFqMacroName(fqname)
                   << " "
                   << fqname
                   << std::endl << std::endl;
        }

        void generateReflector(std::ostream &stream, CXCursor cursor, int indent, int numParams) {
            auto bases = getChildrenOfKind(cursor, CXCursor_CXXBaseSpecifier);
            auto namespaces = getNamespaces(cursor);
            const std::string reflectMethod = "reflect";
            const std::string fqname = getString(clang_getTypeSpelling(clang_getCursorType(cursor)));

            // function definition
            stream << util::indent(indent) << "template<";
            for (int idx = 0; idx < numParams; idx++) {
                stream << "typename T" << idx+1 << ", ";
            }
            stream << "typename F>" << std::endl
                   << util::indent(indent)
                   << "void " 
                   << getFqMacroName(fqname) << "::" << reflectMethod << "(";
            for (int idx = 0; idx < numParams; idx++) {
                stream << "T" << idx+1 << " &o" << idx+1 << ", ";
            }
            stream << "F &f) {"
                   << std::endl;

            // reflect on base-classes
            for (const auto &base : bases) {
                stream << util::indent(indent+1)
                       << getString(clang_getTypeSpelling(clang_getCursorType(base))) << "::"
                       << reflectMethod
                            << "<" << std::endl;
                for (int idx = 0; idx < numParams; idx++) {
                    stream << util::indent(indent+2)
                           << "typename std::conditional<std::is_const<T" << idx+1 << ">::value,"
                                   << "const " << getString(clang_getTypeSpelling(clang_getCursorType(base))) << ","
                                   << getString(clang_getTypeSpelling(clang_getCursorType(base)))
                               << ">::type"
                           << "," << std::endl;
                }
                stream << util::indent(indent+2) << "F" << std::endl
                       << util::indent(indent+1) << ">(";
                for (int idx = 0; idx < numParams; idx++) {
                    stream << "o" << idx+1 << ", ";
                }
                stream << "f);"
                       << std::endl;
            }

            // reflect on each field
            auto fields = getChildrenOfKind(cursor, CXCursor_FieldDecl);
            for (const auto &field : fields) {
                auto spelling = getString(clang_getCursorSpelling(field));
                stream << util::indent(indent+1)
                       << "f("
                       << "\"" << spelling << "\""
                       << ", "
                       << "\"" << getString(clang_getTypeSpelling(clang_getCursorType(field))) << "\""
                       << ", "
                       << "\"" << getString(clang_getCXXAccessSpecifier(field)) << "\""
                       << ", "
                       << "\"" << util::escapeString(getString(clang_Cursor_getBriefCommentText(field))) << "\"";
                for (int idx = 0; idx < numParams; idx++) {
                    stream << ", o" << idx+1 << "." << spelling;
                }
                stream << ");"
                       << std::endl;
            }

            // close out the function
            stream << util::indent(indent)
                   << "}"
                   << std::endl
                   << std::endl;
        }

    protected:
        std::vector<std::string> clangArgs;
    };
}
