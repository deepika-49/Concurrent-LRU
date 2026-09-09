# Concurrent-LRU

Thread-safe LRU Cache implementation in C++17 using a hash map and doubly linked list for O(1) average-time get and put operations.

## Project Overview

This project implements a high-performance, thread-safe Least Recently Used (LRU) Cache in C++17.

The cache uses:
- `std::unordered_map` for O(1) average-time key lookup
- `std::list` as a doubly linked list to maintain LRU ordering
- `std::shared_mutex` for thread-safe access
- RAII-based standard containers for safe memory management

## Features

- O(1) average-time get operation
- O(1) average-time put operation
- Automatic least-recently-used item eviction
- Thread-safe cache operations
- Configurable cache capacity
- Concurrency stress testing
- Unit testing
- Capacity and eviction testing

## Project Structure

Concurrent-LRU/
├── README.md
├── lru_cache.hpp
└── main.cpp

## Technologies

- C++17
- STL
- std::unordered_map
- std::list
- std::shared_mutex
- Multithreading

## Testing

The project includes basic LRU functionality tests, concurrency stress tests using multiple threads, and capacity and eviction tests.

All tests are executed from main.cpp.
