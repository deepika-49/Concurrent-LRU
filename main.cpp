#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <string>
#include "lru_cache.hpp"

void run_unit_tests() {
    ConcurrentLRUCache<int, std::string> cache(2);

    cache.put(1, "alpha");
    cache.put(2, "beta");
    assert(cache.get(1).value() == "alpha");

    cache.put(3, "gamma");
    assert(!cache.get(2).has_value());
    assert(cache.get(3).value() == "gamma");

    std::cout << "Unit tests completed successfully." << std::endl;
}

void run_stress_test() {
    const int num_threads = 8;
    const int ops_per_thread = 20000;
    ConcurrentLRUCache<int, int> cache(200);

    auto worker = [&](int thread_id) {
        for (int i = 0; i < ops_per_thread; ++i) {
            int key = (thread_id * 500) + (i % 300);
            cache.put(key, i);
            cache.get(key);
        }
    };

    std::vector<std::thread> workers;
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker, i);
    }

    for (auto& w : workers) {
        w.join();
    }

    std::cout << "Concurrency stress test completed successfully." << std::endl;
}

int main() {
    run_unit_tests();
    run_stress_test();
    return 0;
}
