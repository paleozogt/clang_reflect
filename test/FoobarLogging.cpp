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

template<typename T>
void log(const T &val) {
    FieldLogger logger;
    T::reflect(val, logger);
    std::cout << std::endl;
}

TEST(foobar, logging) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    log(foobar);
}

