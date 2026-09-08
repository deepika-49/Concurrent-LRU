#pragma once

#include <iostream>
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <mutex>
#include <optional>
#include <utility>
#include <stdexcept>

template <typename KeyType, typename ValueType>
class ConcurrentLRUCache {
public:
    explicit ConcurrentLRUCache(size_t capacity) : capacity_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than zero.");
        }
    }

    std::optional<ValueType> get(const KeyType& key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = map_.find(key);

        if (it == map_.end()) {
            return std::nullopt;
        }

        list_.splice(list_.begin(), list_, it->second);

        return it->second->second;
    }

    void put(const KeyType& key, const ValueType& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = map_.find(key);

        if (it != map_.end()) {
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        if (map_.size() >= capacity_) {
            auto last_it = list_.end();
            --last_it;

            map_.erase(last_it->first);
            list_.pop_back();
        }

        list_.push_front({key, value});
        map_[key] = list_.begin();
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.size();
    }

    size_t capacity() const {
        return capacity_;
    }

private:
    using Node = std::pair<KeyType, ValueType>;
    using ListIterator = typename std::list<Node>::iterator;

    size_t capacity_;
    std::list<Node> list_;
    std::unordered_map<KeyType, ListIterator> map_;
    mutable std::shared_mutex mutex_;
};
