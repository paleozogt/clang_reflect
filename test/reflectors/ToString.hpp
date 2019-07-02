#pragma once

#include <string>
#include <sstream>
#include "reflect.hpp"

class ToString {
public:
    ToString(std::ostream &stream)
        : stream(stream)
    {
    }

    std::ostream &stream;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &comment, const T &field) {
        stream << field << std::endl;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &comment, const T &field) {
        stream << toString(field);
    }

    template<typename T>
    inline static void toStream(const T &val, std::ostream &stream) {
        ToString tostringer(stream);
        T::reflect(val, tostringer);
    }

    template<typename T>
    inline static std::string toString(const T &val) {
        std::ostringstream stream;
        toStream(val, stream);
        return stream.str();
    }
};
