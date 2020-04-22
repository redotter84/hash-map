// Copyright (c) 2020, redotter

#pragma once

#include <algorithm>
#include <list>
#include <tuple>
#include <utility>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
 private:
  using Node = std::pair<const KeyType, ValueType>;
  class TableElement;
  template<class ItemType, class IteratorType>
  class Iterator;

 public:
  using iterator = Iterator<
    std::pair<const KeyType, ValueType>,
    typename std::list<Node>::iterator>;

  using const_iterator = Iterator<
    const std::pair<const KeyType, ValueType>,
    typename std::list<Node>::const_iterator>;

  HashMap(Hash hasher = Hash());

  template<typename ConstructIterator>
  explicit HashMap(
      ConstructIterator begin_iter,
      ConstructIterator end_iter,
      Hash hasher = Hash());

  explicit HashMap(
      std::initializer_list<std::pair<KeyType, ValueType>> init,
      Hash _hasher = Hash());

  HashMap<KeyType, ValueType, Hash>& operator= (
      const HashMap<KeyType, ValueType, Hash>& other);

  size_t size() const;
  bool empty() const;
  Hash hash_function() const;

  void insert(std::pair<KeyType, ValueType> item);
  void erase(KeyType key);
  void clear();

  ValueType& operator[] (KeyType key);
  const ValueType& at(KeyType key) const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  iterator find(KeyType key);
  const_iterator find(KeyType key) const;

 private:
  class TableElement {
   public:
    bool is_used = false;
    bool is_erased = false;
    typename std::list<Node>::iterator iter;
    TableElement() {}
    explicit TableElement(typename std::list<Node>::iterator iter)
        : is_used(true), iter(iter) {}
  };

  template<class ItemType, class IteratorType>
  class Iterator {
   public:
    Iterator() {}
    explicit Iterator(IteratorType iter) : iter(iter) {}
    ItemType& operator* () {
      ItemType* ptr = &(*iter);
      return *ptr;
    }
    ItemType* operator-> () {
      ItemType* ptr = &(*iter);
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

   private:
    IteratorType iter;
  };

  const double MAX_LOAD_FACTOR = 0.75;
  const size_t INIT_SIZE = 3;

  size_t GetHash(KeyType key) const;
  std::pair<size_t, size_t> GetPosition(KeyType key) const;
  void RebuildTable();

  std::vector<TableElement> table_;
  std::list<Node> nodes_;
  std::vector<size_t> elements_indices_;
  Hash hasher_;
  size_t table_size_ = 0;
};

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::GetHash(KeyType key) const {
  const size_t hash = hasher_(key);
  return hash % table_size_;
}

template<class KeyType, class ValueType, class Hash>
inline std::pair<size_t, size_t> HashMap<KeyType, ValueType, Hash>::GetPosition(
    KeyType key) const {
  const size_t hash = GetHash(key);
  size_t table_index = hash;
  while (true) {
    if (table_index >= table_size_) {
      table_index = 0;
    }
    if ((table_[table_index].is_used &&
      !(table_[table_index].iter->first == key)) ||
      table_[table_index].is_erased) {
      ++table_index;
    } else {
      break;
    }
  }
  return std::make_pair(table_index, hash);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::RebuildTable() {
  size_t actual_size = elements_indices_.size();
  double load_factor = (actual_size + 1) * 1.0 / table_size_;
  if (load_factor < MAX_LOAD_FACTOR) {
    return;
  }
  const HashMap<KeyType, ValueType, Hash> copy = *this;
  table_size_ = actual_size * ceil(1 / MAX_LOAD_FACTOR);
  clear();
  for (auto iter = copy.begin(); iter != copy.end(); iter++) {
    insert(*iter);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hasher)
    : hasher_(hasher) {
  clear();
}

template<class KeyType, class ValueType, class Hash>
template<typename ConstructIterator>
inline HashMap<KeyType, ValueType, Hash>::HashMap(
    ConstructIterator begin_iter,
    ConstructIterator end_iter,
    Hash hasher)
    : hasher_(hasher) {
  clear();
  for (ConstructIterator iter = begin_iter; iter != end_iter; iter++) {
    insert(*iter);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(
    std::initializer_list<std::pair<KeyType, ValueType>> init,
    Hash _hasher)
    : hasher_(_hasher) {
  clear();
  for (std::pair<KeyType, ValueType> item : init) {
    insert(item);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>& HashMap<KeyType, ValueType,
                                           Hash>::operator=(
    const HashMap<KeyType, ValueType, Hash>& other) {
  const HashMap<KeyType, ValueType, Hash> copy = other;
  clear();
  hasher_ = copy.hasher_;
  for (auto iter = copy.begin(); iter != copy.end(); iter++) {
    insert(*iter);
  }
  return *this;
}

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
  size_t ans = nodes_.size();
  return ans;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
  return nodes_.empty();
}

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
  return hasher_;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(
    std::pair<KeyType, ValueType> item) {
  RebuildTable();
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(item.first);
  if (table_[table_index].is_used) {
    return;
  }
  nodes_.push_back(Node(item));
  const auto iter = std::prev(nodes_.end());
  table_[table_index] = TableElement(iter);
  elements_indices_.push_back(table_index);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(KeyType key) {
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(key);
  if (!table_[table_index].is_used) {
    return;
  }
  auto iter = table_[table_index].iter;
  nodes_.erase(iter);
  table_[table_index] = TableElement();
  table_[table_index].is_erased = true;
  size_t prv = table_index;
  ++table_index;
  while (true) {
    if (table_index >= table_size_) {
      table_index = 0;
    }
    if (!table_[table_index].is_used) {
      break;
    }
    if (table_index == hash) {
      break;
    }
    const size_t other_hash = GetHash(table_[table_index].iter->first);
    if (other_hash != hash) {
      ++table_index;
      continue;
    }
    table_[prv] = table_[table_index];
    table_[table_index] = TableElement();
    table_[table_index].is_erased = true;
    prv = table_index;
    ++table_index;
  }
}

template<class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](KeyType key) {
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(key);
  if (!table_[table_index].is_used) {
    insert(std::make_pair(key, ValueType()));
  }
  std::tie(table_index, hash) = GetPosition(key);
  return table_[table_index].iter->second;
}

template<class KeyType, class ValueType, class Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(KeyType key) const {
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(key);
  if (!table_[table_index].is_used) {
    throw std::out_of_range("");
  }
  return table_[table_index].iter->second;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
  for (size_t table_index : elements_indices_) {
    table_[table_index] = TableElement();
  }
  table_size_ = std::max(table_size_, INIT_SIZE);
  table_.resize(table_size_);
  elements_indices_.clear();
  nodes_.clear();
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
  return iterator(nodes_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
  return iterator(nodes_.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::begin() const {
  return const_iterator(nodes_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::end() const {
  return const_iterator(nodes_.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType,
                                         Hash>::find(KeyType key) {
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(key);
  if (!table_[table_index].is_used) {
    return iterator(nodes_.end());
  }
  return iterator(table_[table_index].iter);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::find(KeyType key) const {
  size_t table_index, hash;
  std::tie(table_index, hash) = GetPosition(key);
  if (!table_[table_index].is_used) {
    return const_iterator(nodes_.end());
  }
  return const_iterator(table_[table_index].iter);
}
