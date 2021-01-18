# small_function

Allocation-free move-only alternative to `std::function` for C++17.

## Getting Started

The library is header-only. To use it, clone the repo somewhere
and add the `include` directory to your include path.

If you're using CMake, use the FetchContent module.

```cmake
FetchContent_Declare(
    avakar.small_function
    GIT_REPOSITORY https://github.com/avakar/small_function.git
    GIT_TAG master
    GIT_SHALLOW 1
    )
FetchContent_MakeAvailable(avakar.small_function)

target_link_libraries(my_target PUBLIC avakar::small_function)
```

## Usage

Include `<avakar/small_function.h>` and you're good to go.

```cpp
#include <avakar/small_function.h>
using avakar::small_function;

void get_answer(small_function<void(int)> fn)
{
    if (fn)
        fn(42);
}

int main()
{
    get_answer([](int x) {
        // ...
    });
}
```

You mustn't invoke `small_function` while it's empty. Default
constructor will construct an empty object. The object
contextually converts to bool indicating whether it's non-empty.

By default, `small_function` objects are large enough to contain
a function pointer or a lambda with at most one captured pointer.

```cpp
small_function<void()> a = [this] {}; // ok, only one capture
small_function<void()> b = [this, x] {}; // error, lambda too large
small_function<void()> c = &abort; // ok, plain function pointer
```

You can adjust the size and alignment of the internal storage.

```cpp
my_overaligned_type o; // assume sizeof(o) == 32, alignof(o) == 16
small_function<void()> a = [o]{}; // error, lambda too large
small_function<void(), 32, 8> b = [o]{}; // error, after alignment the lambda is too large
small_function<void(), 32, 16> c = [o]{}; // ok
small_function<void(), 40, 8> d = [o]{}; // ok, even after alignment the lambda will fit
```

Zero-sized storage is allowed. Non-capturing lambdas
will fit in those, but function pointers won't.

```cpp
small_function<void(), 0> a = []{}; // ok, zero-sized lambda
small_function<void(), 0> b = [this]{}; // error, lambda too large
small_function<void(), 0> c = &abort; // error, function pointer too large
```

Template parameters can be deduced automatically from an initializer.

```cpp
small_function a = [] { return 42; };
// decltype(a) == small_function<int(), 0, 1>

small_function b = [this] { return 42; };
// decltype(b) == small_function<int(), 8, 8>
```

The function type can include `noexcept`.

```cpp
small_function<void() noexcept> a = []{}; // error, lambda is not noexcept
small_function<void() noexcept> b = []() noexcept {}; // ok
small_function<void()> c = std::move(b); // ok
small_function<void() noexcept> d = std::move(c); // error, weakening exception specification
```

## Reference

```cpp
template <typename R, typename... An, bool ne, size_t size, size_t align>
struct small_function<R(An...) noexcept(ne), size, align>
{
    // Creates the `small_function` object with an empty state.
    // Such object will be falsy and its `operator()` must not be invoked.
    small_function() noexcept;
    
    // Creates the `small_function` object containing `f`.
    // This constructor only  `f` fits in the storage (see below), `f(an...)` is convertible to `R`,
    // and the `noexcept(f(an...))` is compatible with `ne`.
    // The resulting object will be truthy.
    template <typename F>
    small_function(F f) noexcept;

    // Indicates whether the object is non-empty.
    explicit operator bool() const noexcept;

    // Invokes the contained function object.
    // Behavior is undefined if `this` is empty.
    R operator()(An &&... an) noexcept(ne)

    small_function(small_function && o) noexcept;
    small_function & operator=(small_function o) noexcept;
};
```
