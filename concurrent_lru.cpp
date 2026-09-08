#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cassert>

template <typename K, typename V>
class LRUCache {
private:
    size_t capacity;
    using ListIter = typename std::list<std::pair<K, V>>::iterator;
    
    std::list<std::pair<K, V>> items;
    std::unordered_map<K, ListIter> cacheMap;
    mutable std::mutex mtx;

public:
    explicit LRUCache(size_t cap) : capacity(cap) {}

    std::optional<V> get(const K& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            return std::nullopt;
        }
        items.splice(items.begin(), items, it->second);
        return it->second->second;
    }

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = cacheMap.find(key);
        
        if (it != cacheMap.end()) {
            it->second->second = value;
            items.splice(items.begin(), items, it->second);
            return;
        }

        if (cacheMap.size() >= capacity) {
            auto last = items.end();
            --last;
            cacheMap.erase(last->first);
            items.pop_back();
        }

        items.push_front({key, value});
        cacheMap[key] = items.begin();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return cacheMap.size();
    }
};

template <typename K, typename V>
class NaiveSynchronizedMap {
private:
    std::unordered_map<K, V> internalMap;
    mutable std::mutex mtx;

public:
    std::optional<V> get(const K& key) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = internalMap.find(key);
        if (it == internalMap.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx);
        internalMap[key] = value;
    }
};

void run_unit_tests() {
    LRUCache<int, std::string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");
    assert(cache.get(1).value_or("") == "one");

    cache.put(3, "three");
    assert(!cache.get(2).has_value());
    assert(cache.get(3).value_or("") == "three");
    assert(cache.get(1).value_or("") == "one");
}

void run_concurrency_stress_test() {
    LRUCache<int, int> cache(500);
    const int num_threads = 8;
    const int ops_per_thread = 5000;

    std::vector<std::thread> workers;
    for (int t = 0; t < num_threads; ++t) {
        workers.emplace_back([&cache, t, ops_per_thread]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                int key = (t * 100) + (i % 200);
                cache.put(key, i);
                cache.get(key);
            }
        });
    }

    for (auto& w : workers) {
        w.join();
    }
}

void run_benchmark() {
    const int total_ops = 100000;

    LRUCache<int, int> lruCache(1000);
    auto start_lru = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < total_ops; ++i) {
        lruCache.put(i % 500, i);
        lruCache.get(i % 500);
    }
    auto end_lru = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> lru_dur = end_lru - start_lru;

    NaiveSynchronizedMap<int, int> naiveMap;
    auto start_naive = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < total_ops; ++i) {
        naiveMap.put(i % 500, i);
        naiveMap.get(i % 500);
    }
    auto end_naive = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> naive_dur = end_naive - start_naive;

    std::cout << "LRU Execution Time: " << lru_dur.count() << " ms" << std::endl;
    std::cout << "Naive Execution Time: " << naive_dur.count() << " ms" << std::endl;
}

int main() {
    run_unit_tests();
    run_concurrency_stress_test();
    run_benchmark();
    return 0;
}
