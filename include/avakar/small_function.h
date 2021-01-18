#include "../../src/align.h"
#include "../../src/impl.h"
#include "../../src/remove_member_function_pointer.h"

#include <algorithm>
#include <cstddef>

namespace avakar {

// The implementation of `small_function` is delegated to
// `_small_function::impl`. We could place implementation directly
// in `small_function` if we could deduce `noexcept_` from
// `R(An...) noexcept(noexcept_)`, but this is not currently allowed.
//
// Instead, we use `split_noexcept` to decompose the function
// type into `R(An...)` and `noexcept_`, which are then separately passed to
// `_small_function::impl` as template arguments.
//
// We also do some magic with storage and alignment to eliminate waste:
// * align is adjusted to be at least `alignof(_vtable_t *)`,
// * size is adjusted so that there is no padding in the `small_function` object.

namespace _small_function {

template <typename F, std::size_t size, std::size_t align, bool noexcept_>
struct select_impl;

template <typename R, typename... An, std::size_t size, std::size_t align, bool noexcept_>
struct select_impl<R(An...), size, align, noexcept_>
{
	using _vtable_ptr = vtable_t<R, An...> const *;
	static constexpr std::size_t _align = align > alignof(_vtable_ptr)? align: alignof(_vtable_ptr);
	static constexpr std::size_t _size = ((size + sizeof(_vtable_ptr) + _align - 1) & ~(_align - 1)) - sizeof(_vtable_ptr);

	using type = impl<_size, _align, noexcept_, R, An...>;
};



template <typename F>
struct split_noexcept;

template <typename R, typename... An>
struct split_noexcept<R(An...)>
{
	using type = R(An...);
	static constexpr bool noexcept_ = false;
};

template <typename R, typename... An>
struct split_noexcept<R(An...) noexcept>
{
	using type = R(An...);
	static constexpr bool noexcept_ = true;
};



template <typename F, std::size_t size, std::size_t align>
using dispatch_t
	= typename select_impl<
		typename split_noexcept<F>::type,
		size,
		align,
		split_noexcept<F>::noexcept_
	>::type;

}



template <
	typename F,
	std::size_t size = (std::max)(sizeof(void(*)()), sizeof(void *)),
	std::size_t align = (std::max)(alignof(void(*)()), alignof(void *))>
struct small_function
	: _small_function::dispatch_t<F, size, align>
{
	using _small_function::dispatch_t<F, size, align>::dispatch_t;
};

template <typename R, typename... An>
small_function(R(*)(An...)) -> small_function<
	R(An...),
	sizeof(R(*)(An...)),
	alignof(R(*)(An...))>;

template <typename R, typename... An>
small_function(R(*)(An...) noexcept) -> small_function<
	R(An...) noexcept,
	sizeof(R(*)(An...) noexcept),
	alignof(R(*)(An...) noexcept)>;

template <typename F>
small_function(F) -> small_function<
	_small_function::remove_member_function_pointer_t<decltype(&F::operator())>,
	_small_function::real_sizeof_v<F>,
	alignof(F)>;

}

#pragma once
