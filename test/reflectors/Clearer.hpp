#pragma once

#include <string>

class Clearer {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, T &field) const {
        field = T();
    }

    template<typename T>
    inline static void clear(T &val) {
        Clearer clearer;
        T::reflect(val, clearer);
    }
};
