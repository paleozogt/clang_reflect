#pragma once

#include "Foobaz.hpp"

namespace example {
    namespace foo {
        class Foobar : public Foobase {
        public:
            Foobar(const std::string &a = "", uint32_t b = 0, float c = 0,
                   const std::string &d = "", uint32_t e = 0, float f = 0)
                : Foobase(a, b, c), d(d), e(e), f(f),
                  foobaz(a, b, c, d, e, f)
            {
            }

            ~Foobar() override = default;

            REFLECT_DECL();

            std::string getD() const { return d; }
            void setD(const std::string &val) { d = val; }

            uint32_t getE() const { return e; }
            void setE(const uint32_t &val) { e = val; }

            float getF() const { return f; }
            void setF(const float &val) { f = val; }

        protected:
            std::string d;
            uint32_t e = 0;
            float f = 0;

        public:
            Foobaz foobaz;
        };
    }
}
