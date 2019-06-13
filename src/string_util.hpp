#pragma once

#include <string>
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

    std::string indent(size_t indent) {
        return std::string(indent*4, ' ');
    }
}
