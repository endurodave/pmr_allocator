#ifndef _PMR_ALLOCATOR_H
#define _PMR_ALLOCATOR_H

// @see https://github.com/endurodave/pmr_allocator
// David Lafreniere

#include <memory_resource>
#include <cstddef>
#include "xallocator.h"

// pmr_allocator class uses xallocator fixed-block allocator to handle 
// memory requests. xmalloc and xfree are thread-safe. 
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

        // Optionally catch alignment issues within xmalloc. See 
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

#endif