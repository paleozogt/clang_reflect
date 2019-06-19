#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

#include <iostream>

class FieldLogger {
public:
    FieldLogger(const std::string &prefix)
        : prefix(prefix)
    {
    }

    std::string prefix;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field) const {
        std::cout << prefix << (prefix.empty() ? "" : ".")
                  << name << " " << type << " \"" << field << "\"" << std::endl;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field) const {
        log(field, name);
    }

    template<typename T>
    inline static void log(const T &val, const std::string &prefix = "") {
        FieldLogger logger(prefix);
        T::reflect(val, logger);
        std::cout << std::endl;
    }
};

TEST(foobar, logging) {
    example::foo::Foobar foobar("blah", 1, 2, "boop", 3, 4);
    FieldLogger::log(foobar);
}

