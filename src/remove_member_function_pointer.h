namespace avakar::_small_function {

// This is a helper type function to strip the pointer-to-member portion
// of pointers to member functions. This is used in template deduction guides
// when `F` is the type of a function object used as a constructor argument and
// `decltype(&F::operator())` exists (in particular, the overload set contains
// a single member that is not a function template).
//
// This will usually be `R (F::*)(An...)`, which we then strip to `R(An...)`.
//
// In general, the operator can be inherited from one of `F`'s bases
// and can have `const` and `noexcept` modifiers.
// We ignore `volatile` completely as it is deprecated anyway.

template <typename T>
struct remove_member_function_pointer;

template <typename T>
using remove_member_function_pointer_t = typename remove_member_function_pointer<T>::type;



template <typename R, typename... An, typename G>
struct remove_member_function_pointer<R(G:: *)(An...)>
{
	using type = R(An...);
};

template <typename R, typename... An, typename G>
struct remove_member_function_pointer<R(G:: *)(An...) const>
{
	using type = R(An...);
};

template <typename R, typename... An, typename G>
struct remove_member_function_pointer<R(G:: *)(An...) noexcept>
{
	using type = R(An...) noexcept;
};

template <typename R, typename... An, typename G>
struct remove_member_function_pointer<R(G:: *)(An...) const noexcept>
{
	using type = R(An...) noexcept;
};

}

#pragma once
