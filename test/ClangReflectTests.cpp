#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

TEST(clangReflect, reflectable) {
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobase>::value);
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobaz>::value);
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobar>::value);
    ASSERT_FALSE(reflect::is_reflectable<std::string>::value);
}
