#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include "lru_cache.hpp"

void test_basic_ops() {
    ConcurrentLRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");

    assert(cache.get(1).value() == "one");

    cache.put(3, "three");

    assert(!cache.get(2).has_value());
    assert(cache.get(3).value() == "three");

    std::cout << "[PASS] Basic Unit Tests\n";
}

void test_concurrency_stress() {
    const int num_threads = 8;
    const int ops_per_thread = 10000;

    ConcurrentLRUCache<int, int> cache(100);

    auto worker = [&](int id) {
        for (int i = 0; i < ops_per_thread; ++i) {
            int key = (id * 1000) + (i % 200);

            cache.put(key, i);
            cache.get(key);
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "[PASS] Concurrency Stress Test ("
              << num_threads << " threads, "
              << num_threads * ops_per_thread * 2
              << " operations executed cleanly)\n";
}

void test_capacity() {
    ConcurrentLRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    assert(cache.size() == 2);
    assert(!cache.get(1).has_value());
    assert(cache.get(2).has_value());
    assert(cache.get(3).has_value());

    std::cout << "[PASS] Capacity and Eviction Test\n";
}

int main() {
    test_basic_ops();
    test_concurrency_stress();
    test_capacity();

    std::cout << "\nAll tests completed successfully!\n";

    return 0;
}
