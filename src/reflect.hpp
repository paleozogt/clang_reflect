#pragma once

#define REFLECT_DECL(clazz)                                                                                  \
    template<typename T0, typename F,                                                                        \
        typename std::enable_if<std::is_same<clazz,typename std::remove_cv<T0>::type>::value>::type*         \
    >                                                                                                        \
    static void reflect(T0 &o0, F &f);                                                                       \
                                                                                                             \
    template<typename T0, typename T1, typename F,                                                           \
        typename std::enable_if<std::is_same<clazz,typename std::remove_cv<T0>::type>::value>::type*,        \
        typename std::enable_if<std::is_same<clazz,typename std::remove_cv<T1>::type>::value>::type*         \
    >                                                                                                        \
    static void reflect(T0 &o0, T1 &o1, F &f);                                                               \

