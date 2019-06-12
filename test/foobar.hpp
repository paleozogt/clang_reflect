#pragma once

#include <string>
#include <cstdint>

namespace example {
    namespace foo {
        class Foobar {
        public:
            virtual ~Foobar() = default;
    
            template<typename F>
            friend void reflect(Foobar &, const F &);
        
            template<typename F>
            friend void reflect(const Foobar &, F &);
    
            std::string a;
            uint32_t b = 0;
            float c = 0;
        };
    }
}
