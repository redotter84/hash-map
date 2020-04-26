// Copyright (c) 2020, redotter

#pragma once

#include <algorithm>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
 public:
  using KeyValuePair = std::pair<const KeyType, ValueType>;

  using iterator = typename std::list<KeyValuePair>::iterator;
  using const_iterator = typename std::list<KeyValuePair>::const_iterator;

  HashMap(Hash hasher = Hash());

  template<class ConstructIterator>
  explicit HashMap(
      ConstructIterator begin_iter,
      ConstructIterator end_iter,
      Hash hasher = Hash());

  explicit HashMap(
      std::initializer_list<KeyValuePair> init,
      Hash _hasher = Hash());

  HashMap<KeyType, ValueType, Hash>& operator= (
      const HashMap<KeyType, ValueType, Hash>& other);

  size_t size() const;
  bool empty() const;
  Hash hash_function() const;

  void insert(const KeyValuePair& item);
  void erase(const KeyType& key);
  void clear();

  ValueType& operator[] (const KeyType& key);
  const ValueType& at(const KeyType& key) const;

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;

  iterator find(const KeyType& key);
  const_iterator find(const KeyType& key) const;

 private:
  class TableElement {
   public:
    bool is_used = false;
    bool is_erased = false;
    typename std::list<KeyValuePair>::iterator iter;
    size_t hash = 0;
    TableElement() {}
    explicit TableElement(typename std::list<KeyValuePair>::iterator iter)
        : is_used(true), iter(iter) {}
    static TableElement ErasedElement() {
      TableElement erased_element;
      erased_element.is_erased = true;
      return erased_element;
    }
    bool UsedAndEqualsOrErased(const KeyType& key) const {
      return (is_used && !(iter->first == key)) || is_erased;
    }
  };

  class HashAndPosition {
   public:
    size_t hash;
    size_t position;
    HashAndPosition(size_t hash, size_t position)
        : hash(hash), position(position) {}
  };

  const double maxLoadFactor_ = 0.75;
  const size_t initialSize_ = 3;

  size_t GetHash(const KeyType& key) const;
  HashAndPosition GetHashAndPosition(const KeyType& key) const;
  void RebuildTableIfNeeded();

  std::vector<TableElement> table_;
  std::list<KeyValuePair> key_value_pairs_;
  std::vector<size_t> elements_indices_;
  Hash hasher_;
  size_t table_size_ = 0;
  size_t key_value_pairs_size_ = 0;
};

template<class KeyType, class ValueType, class Hash>
size_t HashMap<KeyType, ValueType, Hash>::GetHash(const KeyType& key) const {
  return hasher_(key) % table_size_;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
        Hash>::HashAndPosition HashMap<KeyType, ValueType,
                                       Hash>::GetHashAndPosition(
    const KeyType& key) const {
  const size_t hash = GetHash(key);
  size_t table_index = hash;
  while (true) {
    if (table_index >= table_size_) {
      table_index = 0;
    }
    if (table_[table_index].UsedAndEqualsOrErased(key)) {
      ++table_index;
    } else {
      break;
    }
  }
  return HashAndPosition(hash, table_index);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::RebuildTableIfNeeded() {
  size_t actual_size = elements_indices_.size();
  double load_factor = (actual_size + 1) * 1.0 / table_size_;
  if (load_factor < maxLoadFactor_) {
    return;
  }
  for (size_t table_index : elements_indices_) {
    table_[table_index] = TableElement();
  }
  table_size_ = actual_size * 2;
  table_.resize(table_size_);
  elements_indices_.clear();
  for (auto iter = key_value_pairs_.begin();
       iter != key_value_pairs_.end(); iter++) {
    HashAndPosition key_hash_pos = GetHashAndPosition(iter->first);
    size_t table_index = key_hash_pos.position;
    size_t hash = key_hash_pos.hash;
    table_[table_index] = TableElement(iter);
    table_[table_index].hash = hash;
    elements_indices_.push_back(table_index);
  }
}

template<class KeyType, class ValueType, class Hash>
HashMap<KeyType, ValueType, Hash>::HashMap(Hash hasher)
    : hasher_(hasher) {
  clear();
}

template<class KeyType, class ValueType, class Hash>
template<class ConstructIterator>
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
    std::initializer_list<KeyValuePair> init,
    Hash _hasher)
    : hasher_(_hasher) {
  clear();
  for (const KeyValuePair& item : init) {
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
  return key_value_pairs_size_;
}

template<class KeyType, class ValueType, class Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
  return key_value_pairs_.empty();
}

template<class KeyType, class ValueType, class Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
  return hasher_;
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::insert(
    const KeyValuePair& item) {
  RebuildTableIfNeeded();
  HashAndPosition key_hash_pos = GetHashAndPosition(item.first);
  size_t table_index = key_hash_pos.position;
  size_t hash = key_hash_pos.hash;
  if (table_[table_index].is_used) {
    return;
  }
  key_value_pairs_.push_back(item);
  ++key_value_pairs_size_;
  const auto iter = std::prev(key_value_pairs_.end());
  table_[table_index] = TableElement(iter);
  table_[table_index].hash = hash;
  elements_indices_.push_back(table_index);
}

template<class KeyType, class ValueType, class Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType& key) {
  HashAndPosition key_hash_pos = GetHashAndPosition(key);
  size_t table_index = key_hash_pos.position;
  size_t hash = key_hash_pos.hash;
  if (!table_[table_index].is_used) {
    return;
  }
  auto iter = table_[table_index].iter;
  key_value_pairs_.erase(iter);
  --key_value_pairs_size_;
  table_[table_index] = TableElement::ErasedElement();
  size_t erased_table_index_to_fill = table_index;
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
    const size_t other_hash = table_[table_index].hash;
    if (other_hash != hash) {
      ++table_index;
      continue;
    }
    table_[erased_table_index_to_fill] = table_[table_index];
    table_[table_index] = TableElement::ErasedElement();
    erased_table_index_to_fill = table_index;
    ++table_index;
  }
}

template<class KeyType, class ValueType, class Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](const KeyType& key) {
  HashAndPosition key_hash_pos = GetHashAndPosition(key);
  size_t table_index = key_hash_pos.position;
  if (!table_[table_index].is_used) {
    insert(std::make_pair(key, ValueType()));
  }
  key_hash_pos = GetHashAndPosition(key);
  table_index = key_hash_pos.position;
  return table_[table_index].iter->second;
}

template<class KeyType, class ValueType, class Hash>
const ValueType& HashMap<KeyType, ValueType,
                         Hash>::at(const KeyType& key) const {
  HashAndPosition key_hash_pos = GetHashAndPosition(key);
  size_t table_index = key_hash_pos.position;
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
  table_size_ = std::max(table_size_, initialSize_);
  table_.resize(table_size_);
  elements_indices_.clear();
  key_value_pairs_.clear();
  key_value_pairs_size_ = 0;
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType, Hash>::begin() {
  return iterator(key_value_pairs_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType, Hash>::end() {
  return iterator(key_value_pairs_.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::begin() const {
  return const_iterator(key_value_pairs_.begin());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::end() const {
  return const_iterator(key_value_pairs_.end());
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::iterator HashMap<KeyType, ValueType,
                                         Hash>::find(const KeyType& key) {
  HashAndPosition key_hash_pos = GetHashAndPosition(key);
  size_t table_index = key_hash_pos.position;
  if (!table_[table_index].is_used) {
    return iterator(key_value_pairs_.end());
  }
  return iterator(table_[table_index].iter);
}

template<class KeyType, class ValueType, class Hash>
typename HashMap<KeyType, ValueType,
                 Hash>::const_iterator HashMap<KeyType, ValueType,
                                               Hash>::find(
    const KeyType& key) const {
  HashAndPosition key_hash_pos = GetHashAndPosition(key);
  size_t table_index = key_hash_pos.position;
  if (!table_[table_index].is_used) {
    return const_iterator(key_value_pairs_.end());
  }
  return const_iterator(table_[table_index].iter);
}
