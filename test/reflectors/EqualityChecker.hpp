#pragma once

#include <string>
#include "reflect.hpp"

class FieldEqualityChecker {
public:
    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &comment, const T &field1, const T &field2) {
        result = result && (field1 == field2);
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const std::string &comment, const T &field1, const T &field2) {
        result = result && equals(field1, field2);
    }

    template<typename T>
    inline static bool equals(const T &a, const T &b) {
        FieldEqualityChecker checker;
        T::reflect(a, b, checker);
        return checker.result;
    }

    bool result= true;
};

template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
inline bool operator==(const T &a, const T &b) {
    return FieldEqualityChecker::equals(a, b);
}
