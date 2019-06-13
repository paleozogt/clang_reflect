#pragma once

#include "foobase.hpp"

namespace example {
    namespace foo {
        class Foobar : public Foobase {
        public:
            ~Foobar() override = default;

            REFLECT_DECL(Foobar);

            std::string d;
            uint32_t e = 0;
            float f = 0;
        };
    }
}
