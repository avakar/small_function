#include "align.h"
#include "real_sizeof.h"

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace avakar::_small_function {

template <typename R, typename... An>
struct vtable_t
{
	void (*destroy)(std::byte * storage) noexcept;
	void (*move_destroy)(std::byte * dest_storage, std::byte * src_storage) noexcept;
	R (*invoke)(std::byte * storage, An &&... an);
};



template <std::size_t size, std::size_t align>
struct storage_t
{
	std::byte * data() noexcept
	{
		return _bytes;
	}

private:
	alignas(align) std::byte _bytes[size];
};

template <std::size_t align>
struct storage_t<0, align>
{
	std::byte * data() noexcept
	{
		return reinterpret_cast<std::byte *>(this);
	}
};



inline constexpr bool _fits_in_storage(
	std::size_t obj_size, std::size_t obj_align,
	std::size_t storage_size, std::size_t storage_align)
{
	std::size_t max_pad
		= obj_align > storage_align
		? obj_align - storage_align
		: 0;

	if (storage_size < max_pad)
		return false;

	return storage_size - max_pad >= obj_size;
}

template <std::size_t obj_size, std::size_t obj_align, std::size_t storage_size, std::size_t storage_align>
inline constexpr bool fits_in_storage_v = _fits_in_storage(obj_size, obj_align, storage_size, storage_align);




template <
	std::size_t storage_size,
	std::size_t storage_align,
	bool noexcept_,
	typename R,
	typename... An>
struct impl
	: private storage_t<storage_size, storage_align>
{
	using result_type = R;

	impl() noexcept
		: _vtable(nullptr)
	{
	}

	template <typename F,
		std::enable_if_t<
			(std::is_nothrow_destructible_v<F>
			&& std::is_nothrow_move_constructible_v<F>
			&& (noexcept_? std::is_nothrow_invocable_r_v<R, F &, An...>: std::is_invocable_r_v<R, F &, An...>)
			&& fits_in_storage_v<real_sizeof_v<F>, alignof(F), storage_size, storage_align>), int> = 0>
	impl(F f) noexcept
	{
		static constexpr vtable_t<R, An...> vtable = {
			/*.destroy =*/ [](std::byte * storage) noexcept {
				F & f = *_small_function::realign<storage_align, F>(storage);
				f.~F();
			},
			/*.move_destroy =*/ [](std::byte * dest_storage, std::byte * src_storage) noexcept {
				F & src = *_small_function::realign<storage_align, F>(src_storage);
				new(_small_function::realign<storage_align, F>(dest_storage)) F(std::move(src));
				src.~F();
			},
			/*.invoke =*/ [](std::byte * storage, An &&... an) noexcept(noexcept_) -> R {
				F & f = *_small_function::realign<storage_align, F>(storage);
				return std::invoke(f, std::forward<An>(an)...);
			},
		};

		_vtable = &vtable;
		new(_small_function::realign<storage_align, F>(this->data())) F(std::move(f));
	}

	template <std::size_t size2, std::size_t align2, bool noexcept2,
		std::enable_if_t<(fits_in_storage_v<storage_size, storage_align, size2, align2> && (!noexcept_ || noexcept2)), int> = 0>
	impl(impl<size2, align2, noexcept2, R, An...> && o) noexcept
		: _vtable(o._vtable)
	{
		o._vtable = nullptr;
		if (_vtable)
			_vtable->move_destroy(this->data(), o.data());
	}

	explicit operator bool() const noexcept
	{
		return _vtable != nullptr;
	}

	R operator()(An &&... an) noexcept(noexcept_)
	{
		return _vtable->invoke(this->data(), std::forward<An>(an)...);
	}

	~impl()
	{
		if (_vtable)
			_vtable->destroy(this->data());
	}

	impl(impl && o) noexcept
		: _vtable(o._vtable)
	{
		o._vtable = nullptr;
		if (_vtable)
			_vtable->move_destroy(this->data(), o.data());
	}

	impl & operator=(impl o) noexcept
	{
		if (_vtable)
			_vtable->destroy(this->data());
		if (o._vtable)
			o._vtable->move_destroy(this->data(), o.data());
		_vtable = o._vtable;
		o._vtable = nullptr;
		return *this;
	}

private:
	template <std::size_t, std::size_t, bool, typename, typename...>
	friend struct impl;

	vtable_t<R, An...> const * _vtable;
};




}

#pragma once
