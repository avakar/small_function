#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace avakar::_small_function {

template <std::size_t from, typename T>
std::enable_if_t<(from < alignof(T)), T *> realign(std::byte * p)
{
	auto addr = reinterpret_cast<std::uintptr_t>(p);
	addr = (addr + alignof(T) - 1) & ~(alignof(T) - 1);
	return reinterpret_cast<T *>(addr);
}

template <std::size_t from, typename T>
std::enable_if_t<(from >= alignof(T)), T *> realign(std::byte * p)
{
	return reinterpret_cast<T *>(p);
}

}

#pragma once
