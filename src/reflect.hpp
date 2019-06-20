#pragma once

#include <string>
#include <functional>
#include <type_traits>

#define REFLECT_DECL()                                        \
    template<typename T, typename F>                          \
    static void reflect(T &o, F &f);                          \
                                                              \
    template<typename T1, typename T2, typename F>            \
    static void reflect(T1 &o1, T2 &o2, F &f);                \


namespace reflect {
    class NoopCallback {
    public:
        template <typename A>
        void operator()(const std::string &, const std::string &, const A &a) const {
        }

        template <typename A>
        void operator()(const std::string &, const std::string &, const A &a, const A &b) const {
        }
    };

    template<typename C, typename F = NoopCallback>
    using reflect_fp = decltype(
                            std::function<void(C&, F&)>(
                                &C::template reflect<C,F>
                            )
                        );

    template<typename C, typename C2 = C, typename F = NoopCallback>
    using reflect2_fp = decltype(
                            std::function<void(C&, C2&, F&)>(
                                &C::template reflect<C,C2,F>
                            )
                        );

    template <typename T>
    class is_reflectable {
        template <typename C>
        static std::true_type test(reflect_fp<C>, reflect2_fp<C>);

        template <typename C>
        static std::false_type test(...);

    public:
        enum { value = decltype(test<T>(nullptr, nullptr))::value };
    };
}
