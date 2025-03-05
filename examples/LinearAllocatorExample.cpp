#include "memory/LinearAllocator.hpp"
#include <fmt/core.h>
#include <vector>

/**
 * @brief Example struct to allocate
 */
struct Point3D {
    float x, y, z;

    void print() {
        fmt::print("Point: ({}, {}, {})\n", x, y, z);
    }
};

int main() {
    // Example 1: Using a fixed array as backing buffer
    {
        fmt::print("\n=== Example 1: Fixed array backing buffer ===\n");

        // Create a backing buffer
        constexpr size_t BUFFER_SIZE = 1024;
        unsigned char backing_buffer[BUFFER_SIZE];

        // Initialize the allocator
        auto allocator = memory::LinearAllocator::create(backing_buffer, BUFFER_SIZE);

        // Allocate a Point3D
        auto point_result = allocator.alloc<Point3D> (1);  // Note the space

        if (point_result) {
            Point3D* point = static_cast<Point3D*>(point_result.value());
            point->x = 1.0f;
            point->y = 2.0f;
            point->z = 3.0f;
            point->print();
        } else {
            fmt::print("Failed to allocate Point3D: {}\n",
                      static_cast<int>(point_result.error()));
        }

        // Allocate an array of integers
        constexpr size_t NUM_INTS = 10;
        auto ints_result = allocator.alloc(sizeof(int) * NUM_INTS);

        if (ints_result) {
            int* ints = static_cast<int*>(ints_result.value());

            // Initialize the integers
            for (size_t i = 0; i < NUM_INTS; i++) {
                ints[i] = static_cast<int>(i * 10);
            }

            // Print the integers
            fmt::print("Integers: ");
            for (size_t i = 0; i < NUM_INTS; i++) {
                fmt::print("{} ", ints[i]);
            }
            fmt::print("\n");

            // Resize the array to hold more integers
            constexpr size_t NEW_NUM_INTS = 15;
            auto resized_result = allocator.resize(ints, sizeof(int) * NUM_INTS,
                                                 sizeof(int) * NEW_NUM_INTS);

            if (resized_result) {
                int* resized_ints = static_cast<int*>(resized_result.value());

                // Initialize the new integers
                for (size_t i = NUM_INTS; i < NEW_NUM_INTS; i++) {
                    resized_ints[i] = static_cast<int>(i * 10);
                }

                // Print the resized array
                fmt::print("Resized integers: ");
                for (size_t i = 0; i < NEW_NUM_INTS; i++) {
                    fmt::print("{} ", resized_ints[i]);
                }
                fmt::print("\n");
            }
        }

        // Using temporary arena memory
        {
            fmt::print("\nTemporary arena memory scope:\n");
            auto temp = memory::TempArenaMemory::begin(&allocator);

            // Allocate within the temporary scope
            auto temp_data_result = allocator.alloc(100);
            if (temp_data_result) {
                fmt::print("  Allocated 100 bytes of temporary memory\n");

                // The allocation is valid here...
            }

            // End the temporary scope, which resets the allocator state
            temp.end();
            fmt::print("  Temporary scope ended, memory restored to previous state\n");
        }

        // Free all memory
        allocator.free_all();
        fmt::print("All memory freed\n");
    }

    // Example 2: Using dynamic allocation for backing buffer
    {
        fmt::print("\n=== Example 2: Dynamic backing buffer ===\n");

        // Create a dynamic backing buffer
        constexpr size_t BUFFER_SIZE = 1024;
        std::unique_ptr<unsigned char[]> dynamic_buffer =
            std::make_unique<unsigned char[]>(BUFFER_SIZE);

        // Initialize the allocator
        auto allocator = memory::LinearAllocator::create(dynamic_buffer.get(), BUFFER_SIZE);

        // Allocate a vector of Points using our allocator
        constexpr size_t NUM_POINTS = 5;
        auto points_result = allocator.alloc(sizeof(Point3D) * NUM_POINTS);

        if (points_result) {
            Point3D* points = static_cast<Point3D*>(points_result.value());

            // Initialize the points
            for (size_t i = 0; i < NUM_POINTS; i++) {
                points[i].x = static_cast<float>(i);
                points[i].y = static_cast<float>(i * 2);
                points[i].z = static_cast<float>(i * 3);
            }

            // Print the points
            fmt::print("Points:\n");
            for (size_t i = 0; i < NUM_POINTS; i++) {
                points[i].print();
            }
        }

        // Dynamic buffer will be freed when it goes out of scope
    }

    return 0;
}
