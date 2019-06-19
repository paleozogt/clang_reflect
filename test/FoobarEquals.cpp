#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

class FieldEqualityChecker {
public:
    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field1, const T &field2) {
        result = result && (field1 == field2);
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field1, const T &field2) {
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

template<typename T>
bool operator==(const T &a, const T &b) {
    return FieldEqualityChecker::equals(a, b);
}

TEST(foobar, equals) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);
    example::foo::Foobar foobar2("blah", 1, 2, "boop", 3, 4);

    ASSERT_TRUE(foobar1 == foobar2);

    foobar2.foobaz.m = "bip";
    ASSERT_FALSE(foobar1 == foobar2);
}
