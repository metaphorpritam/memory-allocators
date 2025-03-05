#include "memory/DynArray.hpp"
#include "memory/LinearAllocator.hpp"
#include <fmt/core.h>
#include <string>

/**
 * @brief Simple struct to demonstrate complex types in DynArray
 */
struct Person {
    std::string name;
    int age;

    Person() : name("Unknown"), age(0) {}
    Person(const std::string& n, int a) : name(n), age(a) {}

    void print() const {
        fmt::print("{} (age: {})", name, age);
    }
};

/**
 * @brief Test basic operations of DynArray with integers
 */
void test_basic_operations() {
    fmt::print("\n=== Basic Operations Test ===\n");

    // Create a dynamic array of integers
    memory::DynArray<int> numbers;

    // Add some elements
    fmt::print("Adding elements: ");
    for (int i = 0; i < 10; ++i) {
        numbers.push_back(i * 10);
        fmt::print("{} ", i * 10);
    }
    fmt::print("\n");

    // Access elements using different methods
    fmt::print("First element: {}\n", numbers.front().value().get());
    fmt::print("Last element: {}\n", numbers.back().value().get());
    fmt::print("Element at index 5: {}\n", numbers.at(5).value().get());

    // Iterate through the array
    fmt::print("All elements: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Remove some elements
    fmt::print("Removing last element...\n");
    numbers.pop_back();

    fmt::print("Removing element at index 3...\n");
    numbers.erase(3);

    fmt::print("Remaining elements: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Clear the array
    fmt::print("Clearing the array...\n");
    numbers.clear();
    fmt::print("Size after clear: {}\n", numbers.get_size());
}

/**
 * @brief Test initializer list constructor
 */
void test_initializer_list() {
    fmt::print("\n=== Initializer List Test ===\n");

    // Create a dynamic array using initializer list
    memory::DynArray<int> numbers = {1, 2, 3, 4, 5};

    fmt::print("Elements from initializer list: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
}

/**
 * @brief Test resize operation
 */
void test_resize() {
    fmt::print("\n=== Resize Test ===\n");

    memory::DynArray<int> numbers = {1, 2, 3};

    fmt::print("Initial elements: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Resize to larger size
    fmt::print("Resizing to 5 elements with default value 42...\n");
    numbers.resize(5, 42);

    fmt::print("After resize (larger): ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Resize to smaller size
    fmt::print("Resizing to 2 elements...\n");
    numbers.resize(2);

    fmt::print("After resize (smaller): ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
}

/**
 * @brief Test using custom objects
 */
void test_custom_objects() {
    fmt::print("\n=== Custom Objects Test ===\n");

    memory::DynArray<Person> people;

    // Add some people
    people.push_back(Person("Alice", 30));
    people.push_back(Person("Bob", 25));
    people.push_back(Person("Charlie", 35));

    fmt::print("People in the array:\n");
    for (size_t i = 0; i < people.get_size(); ++i) {
        fmt::print("  {}: ", i);
        people[i].print();
        fmt::print("\n");
    }
}

/**
 * @brief Test with custom allocator
 */
void test_custom_allocator() {
    fmt::print("\n=== Custom Allocator Test ===\n");

    // Create a backing buffer for our allocator
    constexpr size_t BUFFER_SIZE = 1024;
    unsigned char backing_buffer[BUFFER_SIZE];

    // Create a linear allocator
    auto allocator = memory::LinearAllocator::create(backing_buffer, BUFFER_SIZE);

    // Create a dynamic array using our custom allocator (not taking ownership)
    memory::DynArray<int> numbers(10, &allocator, false);

    // Add some elements
    for (int i = 0; i < 5; ++i) {
        numbers.push_back(i * 100);
    }

    fmt::print("Elements with custom allocator: ");
    for (const auto& num : numbers) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    fmt::print("Memory used from buffer: {} bytes\n", allocator.curr_offset);
}

/**
 * @brief Test error handling
 */
void test_error_handling() {
    fmt::print("\n=== Error Handling Test ===\n");

    memory::DynArray<int> numbers = {10, 20, 30};

    // Test out of range access
    auto result = numbers.at(5);
    if (result) {
        fmt::print("Element at index 5: {}\n", result.value().get());
    } else {
        fmt::print("Error accessing element at index 5: Out of range\n");
    }

    // Test empty array operations
    memory::DynArray<int> empty_array;
    auto pop_result = empty_array.pop_back();
    if (pop_result) {
        fmt::print("Element popped successfully\n");
    } else {
        fmt::print("Error popping from empty array: Empty array\n");
    }
}

/**
 * @brief Test copy and move operations (Rule of 5)
 */
void test_rule_of_five() {
    fmt::print("\n=== Rule of 5 Test ===\n");

    // Original array
    memory::DynArray<int> original = {1, 2, 3, 4, 5};

    // Test copy constructor
    memory::DynArray<int> copy_constructed(original);
    fmt::print("Copy constructed: ");
    for (const auto& num : copy_constructed) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Test copy assignment
    memory::DynArray<int> copy_assigned;
    copy_assigned = original;
    fmt::print("Copy assigned: ");
    for (const auto& num : copy_assigned) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");

    // Test move constructor
    memory::DynArray<int> move_constructed(std::move(copy_constructed));
    fmt::print("Move constructed: ");
    for (const auto& num : move_constructed) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
    fmt::print("Source after move (should be empty): Size = {}\n",
              copy_constructed.get_size());

    // Test move assignment
    memory::DynArray<int> move_assigned;
    move_assigned = std::move(copy_assigned);
    fmt::print("Move assigned: ");
    for (const auto& num : move_assigned) {
        fmt::print("{} ", num);
    }
    fmt::print("\n");
    fmt::print("Source after move assignment (should be empty): Size = {}\n",
              copy_assigned.get_size());
}

int main() {
    fmt::print("DynArray Example Program\n");
    fmt::print("=======================\n");

    test_basic_operations();
    test_initializer_list();
    test_resize();
    test_custom_objects();
    test_custom_allocator();
    test_error_handling();
    test_rule_of_five();

    return 0;
}
