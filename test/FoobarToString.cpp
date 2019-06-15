#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

#include <iostream>
#include <sstream>

class FieldOutputStreamer {
public:
    FieldOutputStreamer(std::ostream &stream)
        : stream(stream)
    {
    }

    std::ostream &stream;

    template<typename T>
    void operator()(const std::string &name, const std::string &type, const T &field) {
        stream << field << std::endl;
    }
};

class FieldInputStreamer {
public:
    FieldInputStreamer(std::istream &stream)
            : stream(stream)
    {
    }

    std::istream &stream;

    template<typename T>
    void operator()(const std::string &name, const std::string &type, T &field) {
        std::string line;
        std::getline(stream, line);

        std::istringstream input(line);
        input >> field;
    }
};

template<>
inline void FieldInputStreamer::operator()(const std::string &name, const std::string &type, std::string &field) {
    std::getline(stream, field);
}

template<typename T>
inline std::string toString(const T &val) {
    std::ostringstream stream;
    FieldOutputStreamer tostringer(stream);
    T::reflect(val, tostringer);
    return stream.str();
}

template<typename T>
inline T fromString(const std::string &str) {
    std::istringstream inputStream(str);
    FieldInputStreamer fromstringer(inputStream);
    T val;
    T::reflect(val, fromstringer);
    return val;
}

TEST(foobar, tostring) {
    std::string str = toString(example::foo::Foobar("blah", 1, 2, "boop", 3, 4));
    ASSERT_EQ("blah\n1\n2\nboop\n3\n4\n", str);
    std::cout << str << std::endl;

    auto foobar = fromString<example::foo::Foobar>(str);
    ASSERT_EQ("blah\n1\n2\nboop\n3\n4\n", toString(foobar));
}
