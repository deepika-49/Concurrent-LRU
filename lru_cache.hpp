#ifndef CONCURRENT_LRU_CACHE_HPP
#define CONCURRENT_LRU_CACHE_HPP

#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <optional>
#include <utility>
#include <stdexcept>

template <typename K, typename V>
class ConcurrentLRUCache {
public:
    explicit ConcurrentLRUCache(size_t capacity) : cap_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than zero");
        }
    }

    std::optional<V> get(const K& key) {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    void put(const K& key, const V& value) {
        std::unique_lock<std::shared_mutex> lock(mtx_);
        auto it = map_.find(key);

        if (it != map_.end()) {
            it->second->second = value;
            items_.splice(items_.begin(), items_, it->second);
            return;
        }

        if (map_.size() >= cap_) {
            auto last = items_.end();
            --last;
            map_.erase(last->first);
            items_.pop_back();
        }

        items_.push_front({key, value});
        map_[key] = items_.begin();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mtx_);
        return map_.size();
    }

    size_t capacity() const {
        return cap_;
    }

private:
    using Pair = std::pair<K, V>;
    using ListIt = typename std::list<Pair>::iterator;

    size_t cap_;
    std::list<Pair> items_;
    std::unordered_map<K, ListIt> map_;
    mutable std::shared_mutex mtx_;
};

#endif
