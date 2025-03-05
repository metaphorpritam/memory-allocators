#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <string_view>
#include <memory>

namespace memory {

/**
 * @brief Error codes for allocator operations
 */
enum class AllocatorError {
    OutOfMemory,      ///< Not enough memory available for allocation
    InvalidAlignment, ///< Alignment value is not a power of two
    NullPointer,      ///< Provided pointer is null
    OutOfBounds       ///< Memory operation outside allocator's buffer
};

/**
 * @brief Linear (Arena) allocator implementation
 *
 * A linear allocator allocates memory in a continuous fashion without
 * support for individual deallocation. Memory is typically freed all at once.
 * Also known as Arena or Region-based allocator.
 *
 * @note This allocator has O(1) allocation complexity.
 */
struct LinearAllocator {
    unsigned char* buf;       ///< Pointer to backing buffer
    size_t buf_len;           ///< Length of backing buffer
    size_t prev_offset;       ///< Previous allocation offset
    size_t curr_offset;       ///< Current allocation offset

    /**
     * @brief Default alignment for allocations
     */
    static constexpr size_t DEFAULT_ALIGNMENT = 2 * sizeof(void*);

    /**
     * @brief Initialize a linear allocator with provided backing buffer
     *
     * @param backing_buffer Pointer to pre-allocated memory buffer
     * @param backing_buffer_length Size of the backing buffer in bytes
     * @return Initialized LinearAllocator
     */
    static LinearAllocator create(void* backing_buffer, size_t backing_buffer_length);

    /**
     * @brief Check if a value is a power of two
     *
     * @param x Value to check
     * @return true if x is a power of two, false otherwise
     */
    static bool is_power_of_two(uintptr_t x);

    /**
     * @brief Align a memory address forward to the specified alignment
     *
     * @param ptr Address to align
     * @param align Alignment (must be a power of two)
     * @return std::expected<uintptr_t, AllocatorError> Aligned address or error
     */
    static std::expected<uintptr_t, AllocatorError> align_forward(uintptr_t ptr, size_t align);

    /**
     * @brief Allocate memory with alignment
     *
     * @param size Size in bytes to allocate
     * @param align Alignment of the allocation (must be a power of two)
     * @return std::expected<void*, AllocatorError> Pointer to allocated memory or error
     *
     * @note Allocated memory is zeroed by default
     */
    std::expected<void*, AllocatorError> alloc_align(size_t size, size_t align);

    /**
     * @brief Allocate memory with default alignment
     *
     * @param size Size in bytes to allocate
     * @return std::expected<void*, AllocatorError> Pointer to allocated memory or error
     */
    std::expected<void*, AllocatorError> alloc(size_t size);

    /**
     * @brief Allocate memory for a specific type with default alignment
     *
     * @tparam T Type to allocate memory for
     * @param count Number of elements to allocate
     * @return std::expected<void*, AllocatorError> Pointer to allocated memory or error
     */
    template<typename T>
    std::expected<void*, AllocatorError> alloc(size_t count = 1) {
        return alloc(sizeof(T) * count);
    }

    /**
     * @brief Resize an existing allocation with alignment
     *
     * @param old_memory Pointer to previously allocated memory
     * @param old_size Previous allocation size
     * @param new_size New allocation size
     * @param align Alignment of the allocation (must be a power of two)
     * @return std::expected<void*, AllocatorError> Pointer to resized memory or error
     *
     * @note If old_memory was the most recent allocation, it will be resized in-place.
     *       Otherwise, a new allocation is made and data is copied.
     */
    std::expected<void*, AllocatorError> resize_align(void* old_memory, size_t old_size,
                                                   size_t new_size, size_t align);

    /**
     * @brief Resize an existing allocation with default alignment
     *
     * @param old_memory Pointer to previously allocated memory
     * @param old_size Previous allocation size
     * @param new_size New allocation size
     * @return std::expected<void*, AllocatorError> Pointer to resized memory or error
     */
    std::expected<void*, AllocatorError> resize(void* old_memory, size_t old_size, size_t new_size);

    /**
     * @brief Free a specific memory allocation
     *
     * @param ptr Pointer to memory previously allocated by this allocator
     * @return std::expected<void, AllocatorError> Success or error code
     *
     * @note This function does nothing in a linear allocator and exists for interface completeness
     */
    std::expected<void, AllocatorError> free(void* ptr);

    /**
     * @brief Free all allocations from this allocator
     */
    void free_all();
};

/**
 * @brief Temporary arena memory scope for short-lived allocations
 *
 * This struct can be used to save and restore the state of a linear allocator,
 * allowing temporary allocations to be automatically freed when going out of scope.
 */
struct TempArenaMemory {
    LinearAllocator* arena;  ///< The arena to save/restore
    size_t prev_offset;      ///< Saved previous offset
    size_t curr_offset;      ///< Saved current offset

    /**
     * @brief Begin a temporary memory scope from the given arena
     *
     * @param a Pointer to the arena to create a temporary scope from
     * @return TempArenaMemory Temporary memory scope object
     */
    static TempArenaMemory begin(LinearAllocator* a);

    /**
     * @brief End the temporary memory scope, restoring the arena to its previous state
     */
    void end();
};

} // namespace memory
