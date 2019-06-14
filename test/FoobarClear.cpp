#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

class FieldClearer {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, T &field) const {
        field = T();
    }
};

TEST(foobar, clear) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    ASSERT_EQ("blah", foobar.a);
    example::foo::reflect(foobar, FieldClearer());
    ASSERT_EQ("", foobar.a);
}
