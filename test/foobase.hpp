#pragma once

#include "reflect.hpp"

#include <string>
#include <cstdint>

namespace example {
    namespace foo {
        class Foobase {
        public:
            Foobase(const std::string &a= "", uint32_t b = 0, float c = 0)
                : a(a), b(b), c(c)
            {
            }

            virtual ~Foobase() = default;

            REFLECT_DECL(Foobase);

            std::string a;
            uint32_t b = 0;
            float c = 0;
        };
    }
}
