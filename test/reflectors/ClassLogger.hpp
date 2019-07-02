#pragma once

#include <iostream>
#include <string>
#include "reflect.hpp"

class ClassLogger {
public:
    ClassLogger(const std::string &prefix)
        : prefix(prefix)
    {
    }

    std::string prefix;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, const T &field) const {
        std::cout << prefix << (prefix.empty() ? "" : ".") << name << std::endl
                  << "\t type   : " << type << std::endl
                  << "\t access : " << access << std::endl
                  << "\t comment: " << comment << std::endl
                  << "\t value  : " << field << std::endl
                  << std::endl;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, const T &field) const {
        log(field, name);
    }

    template<typename T>
    inline static void log(const T &val, const std::string &prefix = "") {
        ClassLogger logger(prefix);
        T::reflect(val, logger);
        std::cout << std::endl;
    }
};