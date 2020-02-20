// Copyright (c) 2020, redotter

#pragma once

#include <algorithm>
#include <list>
#include <vector>
#include <tuple>
#include <utility>

const size_t TABLE_SIZE = 3e6;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
  private:
    class Node {
      public:
        std::pair<const KeyType, ValueType> item;

        Node() {}

        Node(std::pair<KeyType, ValueType> item)
            : item(std::make_pair(item.first, item.second)) {}
    };

    class Vertex {
      public:
        bool is_used = false;
        bool is_erased = false;
        typename std::list<Node>::iterator iter;

        Vertex()
        : is_used(false),
          is_erased(false) {}

        explicit Vertex(typename std::list<Node>::iterator iter)
        : is_used(true),
          is_erased(false),
          iter(iter) {}
    };


    template<class ItemType, class IteratorType>
    class Iterator {
      private:
        IteratorType iter;

      public:
        Iterator() {}

        explicit Iterator(IteratorType iter)
        : iter(iter) {}

        ItemType& operator* () {
            ItemType* ptr = &(iter->item);
            return *ptr;
        }

        ItemType* operator-> () {
            ItemType* ptr = &(iter->item);
            return ptr;
        }

        Iterator operator++ (int stuff) {
            Iterator temp = *this;
            iter = std::next(iter);
            return temp;
        }

        Iterator operator++ () {
            iter = std::next(iter);
            return *this;
        }

        bool operator== (const Iterator& other) {
            return iter == other.iter;
        }

        bool operator!= (const Iterator& other) {
            return iter != other.iter;
        }
    };

    Vertex table_[TABLE_SIZE];
    std::list<Node> nodes_;
    std::vector<size_t> vertices_;
    Hash hasher_;

    size_t get_hash(KeyType key) const {
        const size_t hash = hasher_(key);
        return hash % TABLE_SIZE;
    }

    std::pair<size_t, size_t> get_position(KeyType key) const {
        const size_t hash = get_hash(key);
        size_t pos = hash;
        while (true) {
            if (pos >= TABLE_SIZE) {
                pos = 0;
            }
            if ((table_[pos].is_used && !(table_[pos].iter->item.first == key))
                || table_[pos].is_erased) {
                ++pos;
            } else {
                break;
            }
        }
        return std::make_pair(pos, hash);
    }

  public:
    using iterator = Iterator<
        std::pair<const KeyType, ValueType>,
        typename std::list<Node>::iterator>;

    using const_iterator = Iterator<
        const std::pair<const KeyType, ValueType>,
        typename std::list<Node>::const_iterator>;

    HashMap(Hash hasher = Hash())
    : hasher_(hasher) {
        clear();
    }

    template<typename ConstructIterator>
    explicit HashMap(
        ConstructIterator begin_iter,
        ConstructIterator end_iter,
        Hash hasher = Hash()
    )
    : hasher_(hasher) {
        clear();
        for (ConstructIterator iter = begin_iter; iter != end_iter; iter++) {
            insert(*iter);
        }
    }

    explicit HashMap(
        std::initializer_list<std::pair<KeyType, ValueType>> init,
        Hash _hasher = Hash()
    )
    : hasher_(_hasher) {
        clear();
        for (std::pair<KeyType, ValueType> item : init) {
            insert(item);
        }
    }

    HashMap<KeyType, ValueType, Hash>& operator= (
        const HashMap<KeyType, ValueType, Hash>& other
    ) {
        const HashMap<KeyType, ValueType, Hash> copy = other;
        clear();
        hasher_ = copy.hasher_;
        for (auto iter = copy.begin(); iter != copy.end(); iter++) {
            insert(*iter);
        }
        return *this;
    }

    size_t size() const {
        return nodes_.size();
    }

    bool empty() const {
        return nodes_.empty();
    }

    Hash hash_function() const {
        return hasher_;
    }

    void insert(std::pair<KeyType, ValueType> item) {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(item.first);
        if (table_[pos].is_used) {
            return;
        }
        nodes_.push_back(Node(item));
        const auto iter = std::prev(nodes_.end());
        table_[pos] = Vertex(iter);
        vertices_.push_back(pos);
    }

    void erase(KeyType key) {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(key);
        if (!table_[pos].is_used) {
            return;
        }
        auto iter = table_[pos].iter;
        nodes_.erase(iter);
        table_[pos] = Vertex();
        table_[pos].is_erased = true;
        size_t prv = pos;
        ++pos;
        while (true) {
            if (pos >= TABLE_SIZE) {
                pos = 0;
            }
            if (!table_[pos].is_used) {
                break;
            }
            if (pos == hash) {
                break;
            }
            const size_t other_hash = get_hash(table_[pos].iter->item.first);
            if (other_hash != hash) {
                ++pos;
                continue;
            }
            table_[prv] = table_[pos];
            table_[pos] = Vertex();
            table_[pos].is_erased = true;
            prv = pos;
            ++pos;
        }
    }

    ValueType& operator[] (KeyType key) {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(key);
        if (!table_[pos].is_used) {
            insert(std::make_pair(key, ValueType()));
        }
        return table_[pos].iter->item.second;
    }

    const ValueType& at(KeyType key) const {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(key);
        if (!table_[pos].is_used) {
            throw std::out_of_range("");
        }
        return table_[pos].iter->item.second;
    }

    void clear() {
        for (size_t pos : vertices_) {
            table_[pos] = Vertex();
        }
        vertices_.clear();
        nodes_.clear();
    }

    iterator begin() {
        return iterator(nodes_.begin());
    }

    iterator end() {
        return iterator(nodes_.end());
    }

    const_iterator begin() const {
        return const_iterator(nodes_.begin());
    }

    const_iterator end() const {
        return const_iterator(nodes_.end());
    }

    iterator find(KeyType key) {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(key);
        if (!table_[pos].is_used) {
            return iterator(nodes_.end());
        }
        return iterator(table_[pos].iter);
    }

    const_iterator find(KeyType key) const {
        size_t pos, hash;
        std::tie(pos, hash) = get_position(key);
        if (!table_[pos].is_used) {
            return const_iterator(nodes_.end());
        }
        return const_iterator(table_[pos].iter);
    }
};
