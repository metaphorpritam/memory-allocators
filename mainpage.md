# Memory Allocators and Data Structures Library

Welcome to the Memory Allocators and Data Structures library documentation. This library provides efficient memory allocation strategies and data structures implemented in modern C++23.

## Overview

This library focuses on providing efficient, low-level memory management tools that can be used as building blocks for higher-level data structures and algorithms. It emphasizes:

- **Performance**: Optimized for speed and memory efficiency
- **Flexibility**: Support for custom allocation strategies
- **Safety**: Modern error handling with `std::expected`
- **Clarity**: Clean interfaces with comprehensive documentation

## Components

### Linear Allocator (Arena Allocator)

The `LinearAllocator` is a simple but powerful memory allocator that provides O(1) allocation time complexity. It allocates memory in a linear fashion from a pre-allocated memory buffer.

Key features:

- Constant-time allocation
- No per-allocation overhead
- Support for memory alignment
- Batch deallocation

```cpp
// Example: Using a LinearAllocator
unsigned char buffer[1024]; // 1KB buffer
auto allocator = memory::LinearAllocator::create(buffer, 1024);

// Allocate memory
auto result = allocator.alloc(100); // Allocate 100 bytes
if (result) {
    void* memory = result.value();
    // Use the memory...
}

// Free all allocations at once
allocator.free_all();
```

### Dynamic Array

The `DynArray<T>` is a template-based dynamic array implementation that provides functionality similar to `std::vector` but with custom allocator support.

Key features:
- Automatic resizing
- Integration with custom allocators
- Modern error handling via `std::expected`
- Full implementation of the Rule of 5
- Support for standard container operations

```cpp
// Example: Using DynArray
memory::DynArray<int> numbers;

// Add elements
for (int i = 0; i < 10; i++) {
    numbers.push_back(i * 10);
}

// Access elements
auto front = numbers.front();
if (front) {
    int value = front.value().get();
    // Use the value...
}

// Iterate through elements
for (const auto& num : numbers) {
    // Process each number...
}
```

## Memory Management Concepts

### Memory Alignment

Memory alignment ensures that data is stored at memory addresses that are multiples of some value. This is crucial for:

- Optimizing memory access performance
- Meeting hardware requirements for certain data types
- Avoiding unaligned memory access penalties

This library handles memory alignment requirements automatically, but also provides explicit control when needed.

### Custom Memory Allocation

Custom allocators allow you to control how memory is allocated and deallocated. Benefits include:

- Reduced fragmentation
- Improved cache locality
- Better performance for specific workloads
- Fine-grained control over memory usage

## Error Handling Strategy

This library uses C++23's `std::expected` for error handling, providing a more expressive and type-safe alternative to exceptions or error codes. This approach:

- Makes error paths explicit in function signatures
- Avoids exception overhead
- Enforces error handling at compile time
- Preserves value semantics

For example:

```cpp
// Allocating memory with error handling
auto result = allocator.alloc(1024);
if (result) {
    // Success path
    void* memory = result.value();
    // Use the memory...
} else {
    // Error path
    AllocatorError error = result.error();
    // Handle the error...
}
```

## Rule of 5 Implementation

The `DynArray<T>` template implements the Rule of 5 for proper resource management:

1. **Destructor**: Ensures proper cleanup of allocated resources
2. **Copy Constructor**: Performs deep copying of elements
3. **Move Constructor**: Efficiently transfers ownership of resources
4. **Copy Assignment Operator**: Combines cleanup and deep copying
5. **Move Assignment Operator**: Combines cleanup and ownership transfer

This ensures that resources are properly managed throughout the lifetime of objects.

## Getting Started

### Building the Library

```bash
mkdir build
cd build
cmake -G Ninja ..
cmake --build .
```

### Using the Library in Your Project

Add the include directory to your include path and link against the memory library:

```cmake
target_include_directories(your_project PRIVATE /path/to/memory/include)
target_link_libraries(your_project PRIVATE memory)
```

## Examples

See the `examples` directory for complete working examples:

- `LinearAllocatorExample.cpp` - Demonstrates LinearAllocator usage
- `DynArrayExample.cpp` - Demonstrates DynArray usage

## License

This library is available under the `do-what-you-want` License. See the LICENSE file for details.

## Author

Pritam Sarkar
