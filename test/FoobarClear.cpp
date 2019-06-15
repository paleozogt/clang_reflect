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

template<typename T>
void clear(T &val) {
    FieldClearer clearer;
    T::reflect(val, clearer);
}

TEST(foobar, clear) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    ASSERT_EQ("blah", foobar.a);
    clear(foobar);
    ASSERT_EQ("", foobar.a);
}
