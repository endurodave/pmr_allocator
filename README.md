![License MIT](https://img.shields.io/github/license/BehaviorTree/BehaviorTree.CPP?color=blue)
[![conan Ubuntu](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_ubuntu.yml/badge.svg)](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_ubuntu.yml)
[![conan Ubuntu](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_clang.yml/badge.svg)](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_clang.yml)
[![conan Windows](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/endurodave/pmr_allocator/actions/workflows/cmake_windows.yml)

# C++ `std::pmr` Fixed-Block Memory Allocator 

A Polymorphic Memory Resource (PMR) fixed-block memory allocator for C++17 and later.

See [STL Allocator](https://github.com/endurodave/stl_allocator) for an alternative implementation that does not rely upon PMR.

# Table of Contents

- [C++ `std::pmr` Fixed-Block Memory Allocator](#c-stdpmr-fixed-block-memory-allocator)
- [Table of Contents](#table-of-contents)
- [Getting Started](#getting-started)
- [References](#references)
- [Introduction](#introduction)
- [`xallocator`](#xallocator)
- [`pmr_allocator`](#pmr_allocator)
  - [Memory Alignment](#memory-alignment)
- [Usage](#usage)
- [Portable C/C++ Libraries](#portable-cc-libraries)


# Getting Started

[CMake](https://cmake.org/) is used to create the project build files on any C++ system including Windows, Linux, or an embedded system. 

1. Clone the repository.
2. From the repository root, run the following CMake command:   
   `cmake -B build .`
3. Build and run the project within the `build` directory. 

# References

- [Allocator](https://github.com/endurodave/Allocator) - An efficient C++ fixed-block memory allocator.
- [xallocator](https://github.com/endurodave/xallocator) - A `malloc`/`free` fixed-block memory allocator replacement.  
- [stl_allocator](https://github.com/endurodave/stl_allocator) - A `std::allocator` compatible fixed-block memory allocator for use with the C++ Standard Library (`std::list`, `std::string`, `std::stringstream`, ...).

# Introduction

C++17 introduced the Polymorphic Memory Resource (PMR) facility to decouple memory allocation from container logic. This project presents a custom `pmr_allocator`, a Polymorphic Memory Resource implementation designed to provide memory from fixed-size block pools.

Unlike general-purpose allocators, which must handle variable-sized requests, fixed-block allocators are optimized for speed and determinism in tightly constrained systems. The result is consistent allocation and deallocation times and improved fault tolerance in long-running applications.

This article describes how to use the `pmr_allocator` to make STL containers safer and more predictable in embedded and real-time environments.

# `xallocator`

Most of the heavy lifting for the new fixed-block STL allocator comes from the underlying `xallocator` described at <a href="https://github.com/endurodave/xallocator">Replace malloc/free with a Fast Fixed Block Memory Allocator</a>. As the title states, this module replaces `malloc`/`free` with new fixed block `xmalloc`/`xfree` versions. `xallocator` is thread-safe for use in a multithreaded application.

To the user, these replacement functions operate in the same way as the standard CRT versions except for the fixed block feature. In short, `xallocator` has three modes of operation: *static pool* and *heap pool*, where all block memory is obtained from pre-declared static or heap memory pools, or *heap blocks* (default), where blocks are obtained from the global heap using `operator new` but recycled for later use when freed. See the aforementioned article for implementation details.

# `pmr_allocator`

The class `prm_allocator` is the fixed-block implementation using `xmalloc` and `xfree` for memory allocate/deallocate.

```cpp
// pmr_allocator class uses xallocator fixed-block allocator to handle 
// memory requests. xallocator is thread-safe. 
class pmr_allocator : public std::pmr::memory_resource {
public:
    pmr_allocator() = default;

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        // xallocator assumes allocations do not exceed alignof(std::max_align_t).
        // Over-aligned types are not supported.
        if (alignment > alignof(std::max_align_t)) {
            throw std::bad_alloc();
        }

        void* p = xmalloc(bytes);

        // Optionally alignment check for issues within xmalloc. See 
        // README.md section Memory Alignment for more info. 
        if (reinterpret_cast<std::uintptr_t>(p) % alignment != 0) {
            xfree(p);
            throw std::runtime_error("xmalloc returned misaligned memory");
        }

        return p;
    }

    void do_deallocate(void* p, std::size_t /*bytes*/, std::size_t /*alignment*/) override {
        xfree(p);
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};
```

## Memory Alignment

The alignment check within `do_allocate` serves as a safeguard to ensure that memory returned by `xmalloc` meets the alignment requirements specified by the caller. While `xallocator` by default uses *heap blocks* mode backed by `operator new`, which guarantees proper alignment for most types, future changes to use a *heap pool* or *static pool* could introduce misaligned allocations. Including this check helps detect such issues early, preventing undefined behavior when allocating over-aligned types. It's a proactive measure to maintain compliance with `std::pmr::memory_resource`'s alignment contract, especially if the underlying allocation strategy changes. See [Allocator](https://github.com/endurodave/Allocator) for information about fixed-block operational modes. 

# Usage

Simple `std::pmr::list` example using `pmr_allocator`.

```cpp
pmr_allocator resource;
std::pmr::polymorphic_allocator<int> alloc(&resource);

std::pmr::list<int> myList(alloc);
myList.push_back(123);
```

To simplify usage of STL containers with a fixed-block `pmr_allocator`, the following helper functions return instances of common C++ containers preconfigured with a `std::pmr::polymorphic_allocator`.

```cpp
std::pmr::string make_pmr_string(const char* s) {
	static pmr_allocator static_resource;
	return std::pmr::string(s, std::pmr::polymorphic_allocator<char>(&static_resource));
}

std::pmr::wstring make_pmr_wstring(const wchar_t* s) {
	static pmr_allocator static_resource;
	return std::pmr::wstring(s, std::pmr::polymorphic_allocator<wchar_t>(&static_resource));
}

template<typename T>
std::pmr::list<T> make_pmr_list() {
	static pmr_allocator static_resource; // one resource per type
	return std::pmr::list<T>(std::pmr::polymorphic_allocator<T>(&static_resource));
}

// etc...
```

Simple examples using helper functions.

```cpp
auto myList = make_pmr_list<int>();
myList.push_back(123);

auto myMap = make_pmr_map<char, int>();
myMap['a'] = 10;

auto myQueue = make_pmr_queue<int>();
myQueue.push(123);

auto mySet = make_pmr_set<std::pmr::string>();
mySet.insert(make_pmr_string("hello"));
mySet.insert(make_pmr_string("world"));
```

# Portable C/C++ Libraries

Explore [portable-c-cpp-libs](https://github.com/endurodave/portable-c-cpp-libs) for reusable C/C++ components, including state machines, callbacks, threading, memory management, communication, fault handling, testing, and more.
