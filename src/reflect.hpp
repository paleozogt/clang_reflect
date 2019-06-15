#pragma once

#define REFLECT_DECL(clazz)                                   \
    template<typename T, typename F>                          \
    static void reflect(T &o, F &f);                          \
                                                              \
    template<typename T1, typename T2, typename F>            \
    static void reflect(T1 &o1, T2 &o2, F &f);                \

