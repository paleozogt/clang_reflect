#pragma once

#include "foobase.hpp"

namespace example {
    namespace foo {
        class Foobar : public Foobase {
        public:
            Foobar(const std::string &a = "", uint32_t b = 0, float c = 0,
                   const std::string &d = "", uint32_t e = 0, float f = 0)
                : Foobase(a, b, c), d(d), e(e), f(f)
            {
            }

            ~Foobar() override = default;

            REFLECT_DECL(Foobar);

            std::string d;
            uint32_t e = 0;
            float f = 0;
        };
    }
}