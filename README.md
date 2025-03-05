# Memory Allocators and Data Structures

A C++ implementation of various memory allocators and data structures with C++23 features.

## Features

- **Linear Allocator (Arena)**: A simple but efficient memory allocator with O(1) allocation complexity
- **Dynamic Array**: A template-based dynamic array implementation with custom allocator support
- **Error Handling**: Modern error handling using C++23's `std::expected`
- **Rule of 5**: Full implementation of the Rule of 5 for proper resource management

## Requirements

- C++23 compatible compiler (GCC 12+, Clang 16+, or MSVC 19.34+)
- CMake 3.15 or newer
- Ninja build system (optional, for faster builds)

## Building the Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G Ninja ..

# Build
cmake --build .
```

## Examples

The project includes examples demonstrating the usage of each component:

- `linear_allocator_example`: Demonstrates the Linear Allocator
- `dyn_array_example`: Demonstrates the Dynamic Array implementation
