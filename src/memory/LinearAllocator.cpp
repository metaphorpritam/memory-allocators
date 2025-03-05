#include "memory/LinearAllocator.hpp"
#include <cstring>

namespace memory {

LinearAllocator LinearAllocator::create(void* backing_buffer, size_t backing_buffer_length) {
    LinearAllocator a;
    a.buf = static_cast<unsigned char*>(backing_buffer);
    a.buf_len = backing_buffer_length;
    a.curr_offset = 0;
    a.prev_offset = 0;
    return a;
}

bool LinearAllocator::is_power_of_two(uintptr_t x) {
    return (x & (x-1)) == 0 && x != 0;
}

std::expected<uintptr_t, AllocatorError> LinearAllocator::align_forward(uintptr_t ptr, size_t align) {
    if (!is_power_of_two(align)) {
        return std::unexpected(AllocatorError::InvalidAlignment);
    }

    uintptr_t p = ptr;
    uintptr_t a = static_cast<uintptr_t>(align);
    // Same as (p % a) but faster as 'a' is a power of two
    uintptr_t modulo = p & (a-1);

    if (modulo != 0) {
        // If 'p' address is not aligned, push the address to the
        // next value which is aligned
        p += a - modulo;
    }
    return p;
}

std::expected<void*, AllocatorError> LinearAllocator::alloc_align(size_t size, size_t align) {
    // Align 'curr_offset' forward to the specified alignment
    uintptr_t curr_ptr = reinterpret_cast<uintptr_t>(buf) + static_cast<uintptr_t>(curr_offset);

    auto aligned_offset_result = align_forward(curr_ptr, align);
    if (!aligned_offset_result) {
        return std::unexpected(aligned_offset_result.error());
    }

    uintptr_t offset = aligned_offset_result.value();
    offset -= reinterpret_cast<uintptr_t>(buf); // Change to relative offset

    // Check to see if the backing memory has space left
    if (offset + size <= buf_len) {
        void* ptr = &buf[offset];
        prev_offset = offset;
        curr_offset = offset + size;

        // Zero new memory by default
        std::memset(ptr, 0, size);
        return ptr;
    }

    // Return error if the arena is out of memory
    return std::unexpected(AllocatorError::OutOfMemory);
}

std::expected<void*, AllocatorError> LinearAllocator::alloc(size_t size) {
    return alloc_align(size, DEFAULT_ALIGNMENT);
}

std::expected<void*, AllocatorError> LinearAllocator::resize_align(void* old_memory, size_t old_size,
                                                                 size_t new_size, size_t align) {
    unsigned char* old_mem = static_cast<unsigned char*>(old_memory);

    if (!is_power_of_two(align)) {
        return std::unexpected(AllocatorError::InvalidAlignment);
    }

    if (old_mem == nullptr || old_size == 0) {
        return alloc_align(new_size, align);
    }
    else if (buf <= old_mem && old_mem < buf + buf_len) {
        if (buf + prev_offset == old_mem) {
            // This was the previous allocation, we can resize in place
            curr_offset = prev_offset + new_size;

            // Check if we're still within buffer bounds
            if (curr_offset > buf_len) {
                return std::unexpected(AllocatorError::OutOfMemory);
            }

            if (new_size > old_size) {
                // Zero the new memory by default
                std::memset(&buf[prev_offset + old_size], 0, new_size - old_size);
            }
            return old_memory;
        }
        else {
            // Not the previous allocation, allocate new memory and copy
            auto new_memory_result = alloc_align(new_size, align);
            if (!new_memory_result) {
                return std::unexpected(new_memory_result.error());
            }

            void* new_memory = new_memory_result.value();
            size_t copy_size = (old_size < new_size) ? old_size : new_size;

            // Copy across old memory to the new memory
            std::memmove(new_memory, old_memory, copy_size);
            return new_memory;
        }
    }
    else {
        // Memory is out of bounds of this allocator
        return std::unexpected(AllocatorError::OutOfBounds);
    }
}

std::expected<void*, AllocatorError> LinearAllocator::resize(void* old_memory, size_t old_size, size_t new_size) {
    return resize_align(old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}

std::expected<void, AllocatorError> LinearAllocator::free(void* ptr) {
    // Linear allocators don't support individual deallocation
    // This function exists for interface completeness
    return {};
}

void LinearAllocator::free_all() {
    curr_offset = 0;
    prev_offset = 0;
}

// TempArenaMemory implementation
TempArenaMemory TempArenaMemory::begin(LinearAllocator* a) {
    TempArenaMemory temp;
    temp.arena = a;
    temp.prev_offset = a->prev_offset;
    temp.curr_offset = a->curr_offset;
    return temp;
}

void TempArenaMemory::end() {
    arena->prev_offset = prev_offset;
    arena->curr_offset = curr_offset;
}

} // namespace memory
