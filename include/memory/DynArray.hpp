#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <expected>
#include <string_view>
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <functional> // For std::reference_wrapper

#include "memory/LinearAllocator.hpp"

namespace memory {

/**
 * @brief Error codes for DynArray operations
 */
enum class DynArrayError {
    OutOfMemory,       ///< Failed to allocate memory
    OutOfRange,        ///< Tried to access an element outside array bounds
    InvalidCapacity,   ///< Invalid capacity requested
    InvalidSize,       ///< Invalid size requested
    EmptyArray         ///< Operation cannot be performed on empty array
};

/**
 * @brief Dynamic array implementation with custom allocator support
 *
 * A dynamic array that can grow as needed and supports custom allocators.
 * Provides standard container operations with error handling via std::expected.
 *
 * @tparam T The type of elements stored in the array
 */
template <typename T>
struct DynArray {
    // Data members first (following data-oriented approach)
    T*              data;        ///< Pointer to the array data
    size_t          size;        ///< Current number of elements
    size_t          capacity;    ///< Current capacity of the array
    LinearAllocator* allocator;  ///< Optional custom allocator
    bool            owns_allocator; ///< Whether we own the allocator (should delete it)

    /**
     * @brief Default growth factor when resizing
     */
    static constexpr float GROWTH_FACTOR = 1.5f;

    /**
     * @brief Default initial capacity if none specified
     */
    static constexpr size_t DEFAULT_CAPACITY = 8;

    /**
     * @brief Default constructor
     *
     * Creates an empty dynamic array with default capacity.
     */
    DynArray();

    /**
     * @brief Constructor with initial capacity
     *
     * @param initial_capacity Initial capacity to allocate
     */
    explicit DynArray(size_t initial_capacity);

    /**
     * @brief Constructor with initial elements
     *
     * @param elements Initializer list of elements
     */
    DynArray(std::initializer_list<T> elements);

    /**
     * @brief Constructor with custom allocator
     *
     * @param alloc Pointer to a LinearAllocator to use
     * @param take_ownership Whether to take ownership of the allocator
     */
    explicit DynArray(LinearAllocator* alloc, bool take_ownership = false);

    /**
     * @brief Constructor with initial capacity and custom allocator
     *
     * @param initial_capacity Initial capacity to allocate
     * @param alloc Pointer to a LinearAllocator to use
     * @param take_ownership Whether to take ownership of the allocator
     */
    DynArray(size_t initial_capacity, LinearAllocator* alloc, bool take_ownership = false);

    /**
     * @brief Destructor
     *
     * Frees all allocated memory and destroys all elements.
     */
    ~DynArray();

    /**
     * @brief Copy constructor
     *
     * @param other DynArray to copy from
     */
    DynArray(const DynArray& other);

    /**
     * @brief Move constructor
     *
     * @param other DynArray to move from
     */
    DynArray(DynArray&& other) noexcept;

    /**
     * @brief Copy assignment operator
     *
     * @param other DynArray to copy from
     * @return Reference to this DynArray
     */
    DynArray& operator=(const DynArray& other);

    /**
     * @brief Move assignment operator
     *
     * @param other DynArray to move from
     * @return Reference to this DynArray
     */
    DynArray& operator=(DynArray&& other) noexcept;

    /**
     * @brief Subscript operator for element access
     *
     * @param index Index of element to access
     * @return Reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    T& operator[](size_t index);

    /**
     * @brief Const subscript operator for element access
     *
     * @param index Index of element to access
     * @return Const reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    const T& operator[](size_t index) const;

    /**
     * @brief Safe element access with error handling
     *
     * @param index Index of element to access
     * @return std::expected<std::reference_wrapper<T>, DynArrayError> Reference to element or error
     */
    std::expected<std::reference_wrapper<T>, DynArrayError> at(size_t index);

    /**
     * @brief Safe const element access with error handling
     *
     * @param index Index of element to access
     * @return std::expected<std::reference_wrapper<const T>, DynArrayError> Const reference to element or error
     */
    std::expected<std::reference_wrapper<const T>, DynArrayError> at(size_t index) const;

    /**
     * @brief Get the first element in the array
     *
     * @return std::expected<std::reference_wrapper<T>, DynArrayError> Reference to first element or error
     */
    std::expected<std::reference_wrapper<T>, DynArrayError> front();

    /**
     * @brief Get the const first element in the array
     *
     * @return std::expected<std::reference_wrapper<const T>, DynArrayError> Const reference to first element or error
     */
    std::expected<std::reference_wrapper<const T>, DynArrayError> front() const;

    /**
     * @brief Get the last element in the array
     *
     * @return std::expected<std::reference_wrapper<T>, DynArrayError> Reference to last element or error
     */
    std::expected<std::reference_wrapper<T>, DynArrayError> back();

    /**
     * @brief Get the const last element in the array
     *
     * @return std::expected<std::reference_wrapper<const T>, DynArrayError> Const reference to last element or error
     */
    std::expected<std::reference_wrapper<const T>, DynArrayError> back() const;

    /**
     * @brief Get direct pointer to the array data
     *
     * @return Pointer to the first element
     */
    T* get_data();

    /**
     * @brief Get const direct pointer to the array data
     *
     * @return Const pointer to the first element
     */
    const T* get_data() const;

    /**
     * @brief Check if the array is empty
     *
     * @return true if empty, false otherwise
     */
    bool empty() const;

    /**
     * @brief Get the current size of the array
     *
     * @return Current number of elements
     */
    size_t get_size() const;

    /**
     * @brief Get the current capacity of the array
     *
     * @return Current capacity
     */
    size_t get_capacity() const;

    /**
     * @brief Reserve memory for at least the specified number of elements
     *
     * @param new_capacity New capacity to reserve
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> reserve(size_t new_capacity);

    /**
     * @brief Resize the array to contain count elements
     *
     * @param count New size of the array
     * @param value Value to initialize new elements with
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> resize(size_t count, const T& value = T());

    /**
     * @brief Shrink capacity to fit the current size
     *
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> shrink_to_fit();

    /**
     * @brief Clear all elements from the array
     *
     * Reduces size to 0 but keeps capacity.
     */
    void clear();

    /**
     * @brief Add an element to the end of the array
     *
     * @param value Element to add
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> push_back(const T& value);

    /**
     * @brief Add an element to the end of the array (move version)
     *
     * @param value Element to add
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> push_back(T&& value);

    /**
     * @brief Remove the last element from the array
     *
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> pop_back();

    /**
     * @brief Insert an element at the specified position
     *
     * @param position Position to insert at (as index)
     * @param value Element to insert
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> insert(size_t position, const T& value);

    /**
     * @brief Erase an element at the specified position
     *
     * @param position Position to erase (as index)
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> erase(size_t position);

    /**
     * @brief Erase a range of elements
     *
     * @param first First position to erase (inclusive)
     * @param last Last position to erase (exclusive)
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> erase_range(size_t first, size_t last);

    // Iterator support
    using iterator = T*;
    using const_iterator = const T*;

    /**
     * @brief Get iterator to the beginning
     *
     * @return Iterator to the first element
     */
    iterator begin();

    /**
     * @brief Get iterator to the end
     *
     * @return Iterator to the element following the last element
     */
    iterator end();

    /**
     * @brief Get const iterator to the beginning
     *
     * @return Const iterator to the first element
     */
    const_iterator begin() const;

    /**
     * @brief Get const iterator to the end
     *
     * @return Const iterator to the element following the last element
     */
    const_iterator end() const;

    /**
     * @brief Get const iterator to the beginning (explicit const version)
     *
     * @return Const iterator to the first element
     */
    const_iterator cbegin() const;

    /**
     * @brief Get const iterator to the end (explicit const version)
     *
     * @return Const iterator to the element following the last element
     */
    const_iterator cend() const;

private:
    /**
     * @brief Internal function to grow the array capacity
     *
     * @param min_capacity Minimum capacity needed
     * @return std::expected<void, DynArrayError> Success or error
     */
    std::expected<void, DynArrayError> grow(size_t min_capacity);
};

} // namespace memory

// Include the implementation
#include "memory/DynArray.tpp"
