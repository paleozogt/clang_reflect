#include "foobar.hpp"
#include "foobarReflect.hpp"

#include "gtest/gtest.h"

#include <iostream>

class FieldLogger {
public:
    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &field) const {
        std::cout << name << " " << type << " " << field << std::endl;
    }
};

TEST(foobar, reflect) {
    example::foo::Foobar foobar;
    example::foo::reflect(foobar, FieldLogger());
}
