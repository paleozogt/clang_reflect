#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

class FieldEqualityChecker {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &field1, const T &field2) {
        equals = equals && (field1 == field2);
    }

    bool equals= true;
};

template<typename T>
bool operator==(const T &a, const T &b) {
    FieldEqualityChecker checker;
    T::reflect(a, b, checker);
    return checker.equals;
}

TEST(foobar, equals) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);
    example::foo::Foobar foobar2("blah", 1, 2, "boop", 3, 4);

    ASSERT_TRUE(foobar1 == foobar2);
}
