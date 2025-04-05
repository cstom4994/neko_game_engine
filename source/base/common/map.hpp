#pragma once

#include "base/common/mem.hpp"
#include "base/common/string.hpp"

#include <utility>
#include <intrin.h>

namespace Neko {

enum class SlotState { Empty, Occupied, Deleted };

template <typename K, typename V>
struct Slot {
    K key;
    V value;
    SlotState state = SlotState::Empty;
};

template <typename K>
struct DefaultHash;

template <>
struct DefaultHash<int> {
    size_t operator()(int key) const noexcept { return static_cast<size_t>(key); }
};

template <>
struct DefaultHash<String> {
    size_t operator()(String key) const noexcept { return static_cast<size_t>(fnv1a(key)); }
};

template <typename K, typename V>
struct KeyValue {
    const K& key;
    V& value;
};

template <typename K, typename V, typename Hash = DefaultHash<K>>
class UnorderedMap {
private:
    Slot<K, V>* slots = nullptr;
    size_t capacity = 0;
    size_t mask = 0;
    size_t count = 0;
    Hash hash_fn;

    static constexpr double LOAD_FACTOR = 0.75;

    size_t probe(size_t index) const { return (index + 1) & mask; }

    static size_t next_power_of_two(size_t n) {
        if (n == 0) return 1;

        unsigned long index;
#if defined(_MSC_VER)
        _BitScanReverse64(&index, static_cast<unsigned long>(n));
#else
        index = 63 - __builtin_clzll(n);
#endif
        return 1ull << (index + 1);
    }

    void rehash(size_t new_capacity) {
        new_capacity = next_power_of_two(new_capacity);
        Slot<K, V>* new_slots = static_cast<Slot<K, V>*>(mem_alloc(new_capacity * sizeof(Slot<K, V>)));
        if (!new_slots) {
            // 处理内存分配失败
            return;
        }

        // 初始化新槽位
        for (size_t i = 0; i < new_capacity; ++i) {
            new_slots[i].state = SlotState::Empty;
        }

        size_t new_mask = new_capacity - 1;

        // 重新插入所有已占用的元素
        for (size_t i = 0; i < capacity; ++i) {
            if (slots[i].state != SlotState::Occupied) continue;

            size_t idx = hash_fn(slots[i].key) & new_mask;
            while (new_slots[idx].state == SlotState::Occupied) {
                idx = probe(idx);
            }
            new_slots[idx].key = std::move(slots[i].key);
            new_slots[idx].value = std::move(slots[i].value);
            new_slots[idx].state = SlotState::Occupied;
        }

        mem_free(slots);
        slots = new_slots;
        capacity = new_capacity;
        mask = new_mask;
    }

public:
    UnorderedMap() { resize(8); }  // 初始容量

    ~UnorderedMap() { trash(); }

    void trash() {
        if (slots) {
            mem_free(slots);
            slots = nullptr;
        }
    }

    // 禁用拷贝和赋值
    UnorderedMap(const UnorderedMap&) = delete;
    UnorderedMap& operator=(const UnorderedMap&) = delete;

    void resize(size_t new_capacity) { rehash(new_capacity); }

    bool insert(const K& key, const V& value) {
        if (count >= capacity * LOAD_FACTOR) {
            resize(capacity * 2);
        }

        size_t index = hash_fn(key) & mask;
        size_t first_deleted = capacity;
        const size_t start_index = index;

        do {
            if (slots[index].state == SlotState::Occupied) {
                if (slots[index].key == key) {
                    slots[index].value = value;
                    return true;
                }
            } else if (slots[index].state == SlotState::Deleted) {
                if (first_deleted == capacity) first_deleted = index;
            } else {
                break;
            }
            index = probe(index);
        } while (index != start_index);

        if (first_deleted != capacity)
            index = first_deleted;
        else if (slots[index].state != SlotState::Empty)
            return false;

        slots[index].key = key;
        slots[index].value = value;
        slots[index].state = SlotState::Occupied;
        ++count;
        return true;
    }

    V* find(const K& key) {
        size_t index = hash_fn(key) & mask;
        const size_t start_index = index;

        do {
            if (slots[index].state == SlotState::Occupied && slots[index].key == key) {
                return &slots[index].value;
            }
            if (slots[index].state == SlotState::Empty) break;
            index = probe(index);
        } while (index != start_index);

        return nullptr;
    }

    bool erase(const K& key) {
        size_t index = hash_fn(key) & mask;
        const size_t start_index = index;

        do {
            if (slots[index].state == SlotState::Occupied && slots[index].key == key) {
                slots[index].state = SlotState::Deleted;
                --count;
                return true;
            }
            if (slots[index].state == SlotState::Empty) break;
            index = probe(index);
        } while (index != start_index);

        return false;
    }

    size_t size() const { return count; }

    V& operator[](const K& key) {
        if (count >= capacity * LOAD_FACTOR) {
            resize(capacity * 2);
        }

        size_t index = hash_fn(key) & mask;
        size_t first_deleted = capacity;
        const size_t start_index = index;

        do {
            if (slots[index].state == SlotState::Occupied && slots[index].key == key) {
                return slots[index].value;
            }
            if (slots[index].state == SlotState::Deleted && first_deleted == capacity) {
                first_deleted = index;
            } else if (slots[index].state == SlotState::Empty) {
                break;
            }
            index = probe(index);
        } while (index != start_index);

        if (first_deleted != capacity) {
            index = first_deleted;
        }

        // 使用 placement new 构造新元素
        new (&slots[index].key) K(key);
        new (&slots[index].value) V{};
        slots[index].state = SlotState::Occupied;
        ++count;
        return slots[index].value;
    }

    // 迭代器实现
    class iterator {
        UnorderedMap* map;
        size_t index;

        void advance() {
            while (index < map->capacity && map->slots[index].state != SlotState::Occupied) {
                ++index;
            }
        }

    public:
        iterator(UnorderedMap* m, size_t i) : map(m), index(i) { advance(); }

        KeyValue<K, V> operator*() { return {map->slots[index].key, map->slots[index].value}; }

        iterator& operator++() {
            ++index;
            advance();
            return *this;
        }

        bool operator!=(const iterator& other) const { return index != other.index; }
    };

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, capacity); }
};

}  // namespace Neko