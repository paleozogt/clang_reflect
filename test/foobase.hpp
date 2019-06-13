#pragma once

#include "reflect.hpp"

#include <string>
#include <cstdint>

namespace example {
    namespace foo {
        class Foobase {
        public:
            virtual ~Foobase() = default;

            REFLECT_DECL(Foobase);

            std::string a;
            uint32_t b = 0;
            float c = 0;
        };
    }
}
