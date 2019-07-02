#pragma once

#include <string>

class Copier {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, const T &from, T &to) const {
        to = from;
    }

    template<typename T>
    inline static void copy(const T &from, T &to) {
        Copier copier;
        T::reflect(from, to, copier);
    }
};
