#include "reflectors/ClassLogger.hpp"
#include "reflectors/Clearer.hpp"
#include "reflectors/Copier.hpp"
#include "reflectors/EqualityChecker.hpp"
#include "reflectors/FromString.hpp"
#include "reflectors/ToString.hpp"

#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

TEST(reflect, is_reflectable) {
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobase>::value);
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobaz>::value);
    ASSERT_TRUE(reflect::is_reflectable<example::foo::Foobar>::value);
    ASSERT_FALSE(reflect::is_reflectable<std::string>::value);
}

TEST(reflect, logging) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    ClassLogger::log(foobar);
}

TEST(reflect, equals) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);
    example::foo::Foobar foobar2("blah", 1, 2, "boop", 3, 4);
    ASSERT_TRUE(foobar1 == foobar2);

    foobar2.foobaz.m = "bip";
    ASSERT_FALSE(foobar1 == foobar2);
}

TEST(reflect, clear) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    ASSERT_FALSE(foobar == example::foo::Foobar());

    Clearer::clear(foobar);
    ASSERT_TRUE(foobar == example::foo::Foobar());
}

TEST(reflect, copy) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);

    example::foo::Foobar foobar2;
    ASSERT_FALSE(foobar1 == foobar2);

    Copier::copy(foobar1, foobar2);
    ASSERT_TRUE(foobar1 == foobar2);
}

TEST(reflect, tostring) {
    example::foo::Foobar foobar1("blah", 1, 2, "boop", 3, 4);
    std::string str = ToString::toString(foobar1);
    std::cout << str << std::endl;
    EXPECT_EQ("blah\n1\n2\nboop\n3\n4\nblah\n1\n2\nboop\n3\n4\n", str);

    auto foobar2 = FromString::fromString<example::foo::Foobar>(str);
    ASSERT_TRUE(foobar1 == foobar2);
}
