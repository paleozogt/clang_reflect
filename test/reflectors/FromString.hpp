#pragma once

#include <string>
#include <sstream>
#include "reflect.hpp"

class FromString {
public:
    FromString(std::istream &stream)
            : stream(stream)
    {
    }

    std::istream &stream;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, T &field) {
        std::string line;
        std::getline(stream, line);

        std::istringstream input(line);
        input >> field;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, T &field) {
        field = fromStream<T>(stream);
    }

    void operator()(const std::string &name, const std::string &type, const std::string &access, const std::string &comment, std::string &field) {
        std::getline(stream, field);
    }

    template<class T>
    inline static T fromStream(std::istream &stream) {
        FromString fromstringer(stream);
        T val;
        T::reflect(val, fromstringer);
        return val;
    }

    template<class T>
    inline static T fromString(const std::string &str) {
        std::istringstream stream(str);
        return fromStream<T>(stream);
    }
};
