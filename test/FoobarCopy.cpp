#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

class FieldCopyer {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &from, T &to) const {
        to = from;
    }
};

template<typename T>
void copy(const T &from, T &to) {
    FieldCopyer checker;
    reflect::reflect(from, to, checker);
}

TEST(foobar, copy) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);
    example::foo::Foobar foobar2;
    copy(foobar1, foobar2);
}
