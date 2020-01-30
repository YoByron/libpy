#pragma once

#include <memory>
#include <tuple>
#include <type_traits>

#include "libpy/borrowed_ref.h"
#include "libpy/demangle.h"
#include "libpy/detail/api.h"
#include "libpy/detail/python.h"
#include "libpy/exception.h"

namespace py {
namespace detail {
struct buffer_free {
    inline void operator()(Py_buffer* view) {
        if (view) {
            PyBuffer_Release(view);
            delete view;
        }
    }
};
}  // namespace detail

using buffer = std::unique_ptr<Py_buffer, detail::buffer_free>;

template<typename T>
constexpr char buffer_format = '\0';

template<>
constexpr char buffer_format<char> = 'c';

template<>
constexpr char buffer_format<signed char> = 'b';

template<>
constexpr char buffer_format<unsigned char> = 'B';

template<>
constexpr char buffer_format<bool> = '?';

template<>
constexpr char buffer_format<short> = 'h';

template<>
constexpr char buffer_format<unsigned short> = 'H';

template<>
constexpr char buffer_format<int> = 'i';

template<>
constexpr char buffer_format<unsigned int> = 'I';

template<>
constexpr char buffer_format<long> = 'l';

template<>
constexpr char buffer_format<unsigned long> = 'L';

template<>
constexpr char buffer_format<long long> = 'q';

template<>
constexpr char buffer_format<unsigned long long> = 'Q';

template<>
constexpr char buffer_format<float> = 'f';

template<>
constexpr char buffer_format<double> = 'd';

namespace detail {
// clang-format off
template<typename A, typename B>
constexpr bool buffer_format_compatible =
    (std::is_integral_v<A> == std::is_integral_v<B> &&
     std::is_signed_v<A> == std::is_signed_v<B> &&
     sizeof(A) == sizeof(B)) ||
    // special case for char and unsigned char; bytes objects report
    // that they are unsigned but we want to be able to read them into
    // std::string_view
    ((std::is_same_v<A, char> && std::is_same_v<B, unsigned char>) ||
     (std::is_same_v<A, unsigned char> && std::is_same_v<B, char>));
// clang-format on

using buffer_format_types = std::tuple<char,
                                       signed char,
                                       unsigned char,
                                       bool,
                                       short,
                                       unsigned short,
                                       int,
                                       unsigned int,
                                       long,
                                       unsigned long,
                                       long long,
                                       unsigned long long,
                                       float,
                                       double>;

template<typename T, typename U>
struct buffer_compatible_format_chars;

template<typename T, typename Head, typename... Tail>
struct buffer_compatible_format_chars<T, std::tuple<Head, Tail...>> {
    using type = decltype(py::cs::cat(
        std::conditional_t<buffer_format_compatible<T, Head>,
                           py::cs::char_sequence<buffer_format<Head>>,
                           py::cs::char_sequence<>>{},
        typename buffer_compatible_format_chars<T, std::tuple<Tail...>>::type{}));
};

template<typename T>
struct buffer_compatible_format_chars<T, std::tuple<>> {
    using type = py::cs::char_sequence<>;
};
}  // namespace detail

/** Get a Python `Py_Buffer` from the given object.

    @param ob The object to read the buffer from.
    @flags The Python buffer request flags.
    @return buf A smart-pointer adapted `Py_Buffer`.
    @throws An exception is thrown if `ob` doesn't expose the buffer interface.
 */
LIBPY_EXPORT py::buffer get_buffer(py::borrowed_ref<> ob, int flags);

/** Check if a buffer format character is compatible with the given C++ type.

    @tparam T The type to check.
    @param fmt The Python buffer code.
    @return Is the buffer code compatible with `T`?
 */
template<typename T>
bool buffer_type_compatible(char fmt) {
    auto arr = py::cs::to_array(
        typename detail::buffer_compatible_format_chars<T, detail::buffer_format_types>::
            type{});
    for (char c : arr) {
        if (fmt == c) {
            return true;
        }
    }
    return false;
}

/** Check if a buffer's type is compatible with the given C++ type.

    @tparam T The type to check.
    @param buf The buffer to check.
    @return Is the buffer code compatible with `T`?
 */
template<typename T>
bool buffer_type_compatible(const py::buffer& buf) {
    if (!buf->format || std::strlen(buf->format) != 1) {
        return false;
    }
    return buffer_type_compatible<T>(*buf->format);
}
}  // namespace py
