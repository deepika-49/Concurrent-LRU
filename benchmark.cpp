#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include "lru_cache.hpp"

template <typename K, typename V>
class NaiveSyncMap {
public:
    void put(const K& k, const V& v) {
        std::lock_guard<std::mutex> lock(mtx_);
        map_[k] = v;
    }

    bool get(const K& k) {
        std::lock_guard<std::mutex> lock(mtx_);
        return map_.find(k) != map_.end();
    }

private:
    std::unordered_map<K, V> map_;
    std::mutex mtx_;
};

int main() {
    const int iterations = 100000;
    const int thread_count = 8;

    ConcurrentLRUCache<int, int> lru(500);
    NaiveSyncMap<int, int> naive;

    auto start_lru = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> lru_threads;

    for (int t = 0; t < thread_count; ++t) {
        lru_threads.emplace_back([&lru, t, iterations]() {
            for (int i = 0; i < iterations; ++i) {
                lru.put((t * 100) + (i % 200), i);
                lru.get((t * 100) + (i % 200));
            }
        });
    }

    for (auto& th : lru_threads) {
        th.join();
    }
    auto end_lru = std::chrono::high_resolution_clock::now();

    auto start_naive = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> naive_threads;

    for (int t = 0; t < thread_count; ++t) {
        naive_threads.emplace_back([&naive, t, iterations]() {
            for (int i = 0; i < iterations; ++i) {
                naive.put((t * 100) + (i % 200), i);
                naive.get((t * 100) + (i % 200));
            }
        });
    }

    for (auto& th : naive_threads) {
        th.join();
    }
    auto end_naive = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> lru_dur = end_lru - start_lru;
    std::chrono::duration<double, std::milli> naive_dur = end_naive - start_naive;

    std::cout << "LRU Cache Time: " << lru_dur.count() << " ms" << std::endl;
    std::cout << "Naive Sync Map Time: " << naive_dur.count() << " ms" << std::endl;

    return 0;
}
