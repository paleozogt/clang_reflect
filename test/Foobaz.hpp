#pragma once

#include "Foobase.hpp"
#include <iostream>

namespace example {
    namespace foo {
        class Foobaz : public Foobase {
        public:
            Foobaz(const std::string &a = "", uint32_t b = 0, float c = 0,
                   const std::string &m = "", uint32_t n = 0, float o = 0)
                    : Foobase(a, b, c), m(m), n(n), o(o)
            {
            }

            ~Foobaz() override = default;

            REFLECT_DECL();

        public:
            std::string m;
            uint32_t n = 0;
            float o = 0;
        };
    }
}
