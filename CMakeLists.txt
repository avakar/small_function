cmake_minimum_required(VERSION 3.18)

project(avakar.small_function)

if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.19")
set(sources
	include/avakar/small_function.h
	src/align.h
	src/impl.h
	src/real_sizeof.h
	src/remove_member_function_pointer.h
	)
endif()
add_library(avakar.small_function INTERFACE ${sources})
target_compile_features(avakar.small_function INTERFACE cxx_std_17)
target_include_directories(avakar.small_function INTERFACE include)

add_library(avakar::small_function ALIAS avakar.small_function)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
include(CTest)
if (BUILD_TESTING)

add_executable(avakar.small_function.test
	test/test.cpp
	)
target_link_libraries(avakar.small_function.test PUBLIC avakar::small_function)
target_compile_features(avakar.small_function.test PUBLIC cxx_std_20)

add_test(NAME avakar.small_function.test COMMAND avakar.small_function.test)

endif()
endif()
