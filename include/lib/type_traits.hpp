#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

namespace veil::std
{
    struct true_type
    {
        static constexpr bool value = true;
    };

    struct false_type
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct is_pointer : false_type
    {
    };

    template <typename T>
    struct is_pointer<T *> : true_type
    {
    };

    template <typename T>
    struct add_rvalue_reference
    {
        using type = T &&;
    };

    template <typename T>
    typename add_rvalue_reference<T>::type declval() noexcept;

    template <typename From, typename To>
    class is_convertible
    {
    private:
        static void test(To);
        template <typename F>
        static constexpr true_type check(decltype(test(declval<F>())) *);
        template <typename>
        static constexpr false_type check(...);

    public:
        static constexpr bool value = decltype(check<From>(nullptr))::value;
    };
}

#endif