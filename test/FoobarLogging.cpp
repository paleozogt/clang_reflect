#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

#include <iostream>

class FieldLogger {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &field) const {
        std::cout << name << " " << type << " \"" << field << "\"" << std::endl;
    }
};

TEST(foobar, logging) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    const FieldLogger logger;
    reflect::reflect(foobar, logger);
    std::cout << std::endl;
}

