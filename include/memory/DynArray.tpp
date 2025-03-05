#pragma once

#include "memory/DynArray.hpp"

namespace memory {

// Default constructor
template <typename T>
DynArray<T>::DynArray()
    : data(nullptr), size(0), capacity(0), allocator(nullptr), owns_allocator(false) {
    auto result = reserve(DEFAULT_CAPACITY);
    // In a default constructor, we can't really report errors, so we assert
    assert(result.has_value() && "Failed to allocate initial capacity");
}

// Constructor with initial capacity
template <typename T>
DynArray<T>::DynArray(size_t initial_capacity)
    : data(nullptr), size(0), capacity(0), allocator(nullptr), owns_allocator(false) {
    auto result = reserve(initial_capacity);
    assert(result.has_value() && "Failed to allocate initial capacity");
}

// Constructor with initializer list
template <typename T>
DynArray<T>::DynArray(std::initializer_list<T> elements)
    : data(nullptr), size(0), capacity(0), allocator(nullptr), owns_allocator(false) {
    auto result = reserve(elements.size());
    if (result.has_value()) {
        for (const auto& element : elements) {
            // We already reserved space, so this won't fail unless something catastrophic happens
            push_back(element);
        }
    } else {
        assert(false && "Failed to allocate capacity for initializer list");
    }
}

// Constructor with custom allocator
template <typename T>
DynArray<T>::DynArray(LinearAllocator* alloc, bool take_ownership)
    : data(nullptr), size(0), capacity(0), allocator(alloc), owns_allocator(take_ownership) {
    auto result = reserve(DEFAULT_CAPACITY);
    assert(result.has_value() && "Failed to allocate initial capacity with custom allocator");
}

// Constructor with initial capacity and custom allocator
template <typename T>
DynArray<T>::DynArray(size_t initial_capacity, LinearAllocator* alloc, bool take_ownership)
    : data(nullptr), size(0), capacity(0), allocator(alloc), owns_allocator(take_ownership) {
    auto result = reserve(initial_capacity);
    assert(result.has_value() && "Failed to allocate initial capacity with custom allocator");
}

// Destructor - Rule of 5 #1
template <typename T>
DynArray<T>::~DynArray() {
    // Destroy all elements
    for (size_t i = 0; i < size; ++i) {
        data[i].~T();
    }

    // Free memory if we're not using a custom allocator
    if (!allocator) {
        free(data);
    } else if (owns_allocator) {
        delete allocator;
    }

    // Reset all members for safety
    data = nullptr;
    size = 0;
    capacity = 0;
    allocator = nullptr;
    owns_allocator = false;
}

// Copy constructor - Rule of 5 #2
template <typename T>
DynArray<T>::DynArray(const DynArray& other)
    : data(nullptr), size(0), capacity(0), allocator(nullptr), owns_allocator(false) {
    // Reserve capacity for the elements
    auto result = reserve(other.capacity);
    if (result.has_value()) {
        // Copy elements
        for (size_t i = 0; i < other.size; ++i) {
            new (data + i) T(other.data[i]); // Placement new to copy construct
        }
        size = other.size;
    } else {
        assert(false && "Failed to allocate memory in copy constructor");
    }
}

// Move constructor - Rule of 5 #3
template <typename T>
DynArray<T>::DynArray(DynArray&& other) noexcept
    : data(other.data), size(other.size), capacity(other.capacity),
      allocator(other.allocator), owns_allocator(other.owns_allocator) {
    // Reset the source object
    other.data = nullptr;
    other.size = 0;
    other.capacity = 0;
    other.allocator = nullptr;
    other.owns_allocator = false;
}

// Copy assignment operator - Rule of 5 #4
template <typename T>
DynArray<T>& DynArray<T>::operator=(const DynArray& other) {
    if (this != &other) {
        // Clear existing data
        clear();

        // Ensure we have enough capacity
        auto result = reserve(other.capacity);
        if (result.has_value()) {
            // Copy elements
            for (size_t i = 0; i < other.size; ++i) {
                new (data + i) T(other.data[i]); // Placement new to copy construct
            }
            size = other.size;
        } else {
            assert(false && "Failed to allocate memory in copy assignment");
        }
    }
    return *this;
}

// Move assignment operator - Rule of 5 #5
template <typename T>
DynArray<T>& DynArray<T>::operator=(DynArray&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        clear();
        if (!allocator) {
            free(data);
        } else if (owns_allocator) {
            delete allocator;
        }

        // Move resources from other
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        allocator = other.allocator;
        owns_allocator = other.owns_allocator;

        // Reset the source object
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
        other.allocator = nullptr;
        other.owns_allocator = false;
    }
    return *this;
}

// Subscript operator
template <typename T>
T& DynArray<T>::operator[](size_t index) {
    if (index >= size) {
        throw std::out_of_range("DynArray index out of range");
    }
    return data[index];
}

// Const subscript operator
template <typename T>
const T& DynArray<T>::operator[](size_t index) const {
    if (index >= size) {
        throw std::out_of_range("DynArray index out of range");
    }
    return data[index];
}

// Safe element access with error handling
template <typename T>
std::expected<std::reference_wrapper<T>, DynArrayError> DynArray<T>::at(size_t index) {
    if (index >= size) {
        return std::unexpected(DynArrayError::OutOfRange);
    }
    return std::ref(data[index]);
}

// Safe const element access with error handling
template <typename T>
std::expected<std::reference_wrapper<const T>, DynArrayError> DynArray<T>::at(size_t index) const {
    if (index >= size) {
        return std::unexpected(DynArrayError::OutOfRange);
    }
    return std::cref(data[index]);
}

// Get the first element
template <typename T>
std::expected<std::reference_wrapper<T>, DynArrayError> DynArray<T>::front() {
    if (empty()) {
        return std::unexpected(DynArrayError::EmptyArray);
    }
    return std::ref(data[0]);
}

// Get the const first element
template <typename T>
std::expected<std::reference_wrapper<const T>, DynArrayError> DynArray<T>::front() const {
    if (empty()) {
        return std::unexpected(DynArrayError::EmptyArray);
    }
    return std::cref(data[0]);
}

// Get the last element
template <typename T>
std::expected<std::reference_wrapper<T>, DynArrayError> DynArray<T>::back() {
    if (empty()) {
        return std::unexpected(DynArrayError::EmptyArray);
    }
    return std::ref(data[size - 1]);
}

// Get the const last element
template <typename T>
std::expected<std::reference_wrapper<const T>, DynArrayError> DynArray<T>::back() const {
    if (empty()) {
        return std::unexpected(DynArrayError::EmptyArray);
    }
    return std::cref(data[size - 1]);
}

// Get direct pointer to data
template <typename T>
T* DynArray<T>::get_data() {
    return data;
}

// Get const direct pointer to data
template <typename T>
const T* DynArray<T>::get_data() const {
    return data;
}

// Check if array is empty
template <typename T>
bool DynArray<T>::empty() const {
    return size == 0;
}

// Get size
template <typename T>
size_t DynArray<T>::get_size() const {
    return size;
}

// Get capacity
template <typename T>
size_t DynArray<T>::get_capacity() const {
    return capacity;
}

// Reserve memory
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::reserve(size_t new_capacity) {
    if (new_capacity <= capacity) {
        return {}; // Nothing to do
    }

    // Allocate new memory
    T* new_data = nullptr;

    if (allocator) {
        // Use custom allocator
        auto alloc_result = allocator->alloc_align(sizeof(T) * new_capacity, alignof(T));
        if (!alloc_result) {
            return std::unexpected(DynArrayError::OutOfMemory);
        }
        new_data = static_cast<T*>(alloc_result.value());
    } else {
        // Use standard allocator
        new_data = static_cast<T*>(malloc(sizeof(T) * new_capacity));
        if (!new_data) {
            return std::unexpected(DynArrayError::OutOfMemory);
        }
    }

    // Move existing elements to new memory
    for (size_t i = 0; i < size; ++i) {
        new (new_data + i) T(std::move(data[i])); // Placement new with move
        data[i].~T(); // Destroy old object
    }

    // Free old memory if it existed
    if (data) {
        if (!allocator) {
            free(data);
        }
    }

    // Update pointers and capacity
    data = new_data;
    capacity = new_capacity;

    return {};
}

// Resize array
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::resize(size_t count, const T& value) {
    if (count > capacity) {
        // Need to allocate more memory
        auto result = reserve(count);
        if (!result) {
            return result;
        }
    }

    if (count > size) {
        // Initialize new elements
        for (size_t i = size; i < count; ++i) {
            new (data + i) T(value); // Placement new to copy construct
        }
    } else if (count < size) {
        // Destroy excess elements
        for (size_t i = count; i < size; ++i) {
            data[i].~T();
        }
    }

    size = count;
    return {};
}

// Shrink to fit
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::shrink_to_fit() {
    if (size == capacity) {
        return {}; // Already fit
    }

    if (size == 0) {
        // Special case: completely empty
        if (!allocator) {
            free(data);
        }
        data = nullptr;
        capacity = 0;
        return {};
    }

    // Allocate new memory of exact size
    T* new_data = nullptr;

    if (allocator) {
        // Use custom allocator
        auto alloc_result = allocator->alloc_align(sizeof(T) * size, alignof(T));
        if (!alloc_result) {
            return std::unexpected(DynArrayError::OutOfMemory);
        }
        new_data = static_cast<T*>(alloc_result.value());
    } else {
        // Use standard allocator
        new_data = static_cast<T*>(malloc(sizeof(T) * size));
        if (!new_data) {
            return std::unexpected(DynArrayError::OutOfMemory);
        }
    }

    // Move existing elements to new memory
    for (size_t i = 0; i < size; ++i) {
        new (new_data + i) T(std::move(data[i])); // Placement new with move
        data[i].~T(); // Destroy old object
    }

    // Free old memory
    if (!allocator) {
        free(data);
    }

    // Update pointers and capacity
    data = new_data;
    capacity = size;

    return {};
}

// Clear array
template <typename T>
void DynArray<T>::clear() {
    // Destroy all elements
    for (size_t i = 0; i < size; ++i) {
        data[i].~T();
    }
    size = 0;
}

// Push back (copy)
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::push_back(const T& value) {
    if (size >= capacity) {
        // Need to grow the array
        auto result = grow(size + 1);
        if (!result) {
            return result;
        }
    }

    // Construct new element at the end
    new (data + size) T(value); // Placement new to copy construct
    ++size;

    return {};
}

// Push back (move)
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::push_back(T&& value) {
    if (size >= capacity) {
        // Need to grow the array
        auto result = grow(size + 1);
        if (!result) {
            return result;
        }
    }

    // Construct new element at the end
    new (data + size) T(std::move(value)); // Placement new to move construct
    ++size;

    return {};
}

// Pop back
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::pop_back() {
    if (empty()) {
        return std::unexpected(DynArrayError::EmptyArray);
    }

    // Destroy the last element
    data[--size].~T();

    return {};
}

// Insert element
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::insert(size_t position, const T& value) {
    if (position > size) {
        return std::unexpected(DynArrayError::OutOfRange);
    }

    if (size >= capacity) {
        // Need to grow the array
        auto result = grow(size + 1);
        if (!result) {
            return result;
        }
    }

    if (position < size) {
        // Shift elements to make room
        for (size_t i = size; i > position; --i) {
            new (data + i) T(std::move(data[i - 1])); // Move construct at new position
            data[i - 1].~T(); // Destroy original
        }
    }

    // Construct new element at position
    new (data + position) T(value);
    ++size;

    return {};
}

// Erase element
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::erase(size_t position) {
    return erase_range(position, position + 1);
}

// Erase range
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::erase_range(size_t first, size_t last) {
    if (first >= size || last > size || first > last) {
        return std::unexpected(DynArrayError::OutOfRange);
    }

    if (first == last) {
        return {}; // Nothing to erase
    }

    // Number of elements to erase
    size_t count = last - first;

    // Destroy elements in the range
    for (size_t i = first; i < last; ++i) {
        data[i].~T();
    }

    // Shift remaining elements
    for (size_t i = first; i < size - count; ++i) {
        new (data + i) T(std::move(data[i + count])); // Move construct at new position
        data[i + count].~T(); // Destroy original
    }

    // Update size
    size -= count;

    return {};
}

// Begin iterator
template <typename T>
typename DynArray<T>::iterator DynArray<T>::begin() {
    return data;
}

// End iterator
template <typename T>
typename DynArray<T>::iterator DynArray<T>::end() {
    return data + size;
}

// Const begin iterator
template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::begin() const {
    return data;
}

// Const end iterator
template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::end() const {
    return data + size;
}

// Explicit const begin iterator
template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::cbegin() const {
    return data;
}

// Explicit const end iterator
template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::cend() const {
    return data + size;
}

// Internal function to grow the array capacity
template <typename T>
std::expected<void, DynArrayError> DynArray<T>::grow(size_t min_capacity) {
    // Calculate new capacity using growth factor
    size_t new_capacity = capacity == 0 ? DEFAULT_CAPACITY :
                           static_cast<size_t>(capacity * GROWTH_FACTOR) + 1;

    // Ensure it's at least as large as requested
    if (new_capacity < min_capacity) {
        new_capacity = min_capacity;
    }

    // Reserve new capacity
    return reserve(new_capacity);
}

} // namespace memory
