#include <cstddef>
#include <type_traits>

namespace avakar::_small_function {

template <typename T, typename = void>
struct real_sizeof
	: std::integral_constant<std::size_t, sizeof(T)>
{
};

template <typename T>
struct real_sizeof<T, std::enable_if_t<std::is_empty_v<T>>>
	: std::integral_constant<std::size_t, 0>
{
};

template <typename T>
inline constexpr std::size_t real_sizeof_v = real_sizeof<T>::value;

}

#pragma once
