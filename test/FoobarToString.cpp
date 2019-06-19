#include "Foobar.hpp"
#include "FoobarReflect.hpp"

#include "gtest/gtest.h"

#include <iostream>
#include <sstream>

/**
 * FieldWriter/FieldReader define a naive tostring/fromstring serialization scheme.
 */

class FieldWriter {
public:
    FieldWriter(std::ostream &stream)
        : stream(stream)
    {
    }

    std::ostream &stream;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field) {
        stream << field << std::endl;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, const T &field) {
        stream << toString(field);
    }

    template<typename T>
    inline static void toStream(const T &val, std::ostream &stream) {
        FieldWriter tostringer(stream);
        T::reflect(val, tostringer);
    }

    template<typename T>
    inline static std::string toString(const T &val) {
        std::ostringstream stream;
        toStream(val, stream);
        return stream.str();
    }
};

class FieldReader {
public:
    FieldReader(std::istream &stream)
            : stream(stream)
    {
    }

    std::istream &stream;

    template<class T, typename std::enable_if<!reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, T &field) {
        std::string line;
        std::getline(stream, line);

        std::istringstream input(line);
        input >> field;
    }

    template<class T, typename std::enable_if<reflect::is_reflectable<T>::value>::type* = nullptr>
    void operator()(const std::string &name, const std::string &type, T &field) {
        field = fromStream<T>(stream);
    }

    void operator()(const std::string &name, const std::string &type, std::string &field) {
        std::getline(stream, field);
    }

    template<class T>
    inline static T fromStream(std::istream &stream) {
        FieldReader fromstringer(stream);
        T val;
        T::reflect(val, fromstringer);
        return val;
    }

    template<class T>
    inline static T fromString(const std::string &str) {
        std::istringstream stream(str);
        return fromStream<T>(stream);
    }
};

TEST(foobar, tostring) {
    std::string str = FieldWriter::toString(example::foo::Foobar("blah", 1, 2, "boop", 3, 4));
    std::cout << str << std::endl;
    EXPECT_EQ("blah\n1\n2\nboop\n3\n4\nblah\n1\n2\nboop\n3\n4\n", str);

    auto foobar = FieldReader::fromString<example::foo::Foobar>(str);
    ASSERT_EQ("blah\n1\n2\nboop\n3\n4\nblah\n1\n2\nboop\n3\n4\n", FieldWriter::toString(foobar));
}
