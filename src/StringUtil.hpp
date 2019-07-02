#pragma once

#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

namespace util {
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

    inline std::string replace(std::string str, const std::string &findstr, const std::string &replstr) {
        size_t pos = str.find(findstr);
        while (pos != std::string::npos) {
            str.replace(pos, findstr.size(), replstr);
            pos = str.find(findstr, pos + replstr.size());
        }
        return str;
    }

    inline std::string escapeString(std::string str) {
        str = replace(str, "\\", "\\\\");
        str = replace(str, "\"", "\\\"");
        str = replace(str, "\n", "\\n");
        str = replace(str, "\r", "\\r");
        str = replace(str, "\t", "\\t");
        return str;
    }

    inline std::vector<std::string> split(const std::string &str, char token= ',') {
        std::istringstream ss(str);
        std::string item;
        std::vector<std::string> vec;
        while (std::getline(ss, item, token)) {
            vec.push_back(item);
        }
        return vec;
    }

    inline std::string indent(size_t indent) {
        return std::string(indent*4, ' ');
    }
}
