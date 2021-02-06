#include <avakar/small_function.h>

#include <stdexcept>
#ifndef PP_STRINGIFY
#define PP_STRINGIFY2(x) #x
#define PP_STRINGIFY(x) PP_STRINGIFY2(x)
#endif
struct assertion_failed
	: std::runtime_error
{
	using runtime_error::runtime_error;
};
#define require(cond) do { if (!(cond)) throw ::assertion_failed(__FILE__ "(" PP_STRINGIFY(__LINE__) "): assertion failed: " #cond); } while (0)

using avakar::small_function;

static void test_empty()
{
	small_function<void()> fn;
	require(!fn);

	fn = [] {};
	require(fn);
}

static void test_return()
{
	small_function<int()> fn = [] { return 42; };
	require(fn);
	require(fn() == 42);
}

void test_args()
{
	small_function<int(int)> fn = [](int a) { return a; };
	require(fn);
	require(fn(45) == 45);
}

void test_default_args()
{
	small_function<int()> fn = [](int a = 44) { return a; };
	require(fn);
	require(fn() == 44);
}

static void test_move_construct()
{
	small_function<void()> fn = [] {};
	require(fn);

	small_function<void()> fn2 = std::move(fn);
	require(!fn);
	require(fn2);
}

static void test_move_assign()
{
	small_function<void()> fn = [] {};
	small_function<void()> fn2;

	require(fn);
	require(!fn2);

	fn2 = std::move(fn);

	require(!fn);
	require(fn2);
}

static void test_size_conversion()
{
	small_function<int(), 0> fn = [] { return 46; };
	require(fn() == 46);

	small_function<int(), 16> fn2 = std::move(fn);
	require(fn2() == 46);
}

static void test_noexcept()
{
	small_function<void()> fn;
	require(!noexcept(fn()));

	small_function<void() noexcept> fn2;
	require(noexcept(fn2()));

}

static void test_noexcept_convertion()
{
	small_function<int()> fn;
	small_function<int() noexcept> fn2 = []() noexcept { return 42; };
	require(fn2);
	require(fn2() == 42);

	fn = std::move(fn2);
	require(fn);
	require(!fn2);
	require(fn() == 42);
}

static int _noexcept_fn() noexcept
{
	return 43;
}

static void test_deduction()
{
	small_function fn = &test_deduction;
	static_assert(std::is_same_v<decltype(fn), small_function<void()>>);

	small_function fn2 = &_noexcept_fn;
	static_assert(std::is_same_v<decltype(fn2), small_function<int() noexcept>>);
	require(fn2() == 43);

	small_function fn3 = [] {};
	static_assert(std::is_same_v<decltype(fn3), small_function<void(), 0, 1>>);
	static_assert(std::is_convertible_v<decltype(fn3), small_function<void()>>);

	small_function fn4 = []() noexcept {};
	static_assert(std::is_same_v<decltype(fn4), small_function<void() noexcept, 0, 1>>);
	static_assert(std::is_convertible_v<decltype(fn4), small_function<void() noexcept>>);
	static_assert(std::is_convertible_v<decltype(fn4), small_function<void()>>);

	small_function fn5 = std::move(fn2);
	static_assert(std::is_same_v<decltype(fn5), decltype(fn2)>);
}

static void test_default_align()
{
	//require(std::is_same_v<small_function<void(), 2>
}

struct alignas(16) _overaligned
{
	int value;
};

static void test_realign()
{
	_overaligned v{ 50 };
	small_function<_overaligned(), 24, 8> fn = [v] { return v; };
	require(fn().value == 50);
}

static void test_pass_lvalue()
{
	small_function<int (int)> fn = [](int x) { return x; };

	int x = 1;
	require(fn(x) == 1);

	small_function<int &(int &)> fn2 = [](int & x) -> int & { return x; };
	require(&fn2(x) == &x);
}

#include <iostream>
int main()
{
	try
	{
		test_empty();
		test_return();
		test_args();
		test_default_args();
		test_move_construct();
		test_move_assign();
		test_size_conversion();
		test_noexcept();
		test_noexcept_convertion();
		test_deduction();
		test_realign();
		test_pass_lvalue();
	}
	catch (assertion_failed const & e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}

	return 0;
}
