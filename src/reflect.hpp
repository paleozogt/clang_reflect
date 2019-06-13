#pragma once

#define REFLECT_DECL(clazz)                           \
    template<typename F>                              \
    friend void reflect(clazz &, const F &);          \
                                                      \
    template<typename F>                              \
    friend void reflect(const clazz &, F &);          \
                                                      \
    template<typename F>                              \
    friend void reflect(const clazz &, const F &);    \
                                                      \
    template<typename F>                              \
    friend void reflect(clazz &, F &);                \

