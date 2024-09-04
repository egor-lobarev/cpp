#ifndef UNORDERED_SET_UNORDERED_SET_H
#define UNORDERED_SET_UNORDERED_SET_H

#include <iostream>
#include <memory>
#include <iterator>
#include <stdexcept>
#include <exception>
#include <vector>
#include <list>
#include <functional>

template <class Key>
class Iterator {
 public:
  typename std::vector<std::list<Key>>::iterator iterator_vector_;
  const typename std::vector<std::list<Key>>::iterator iterator_vector_end_;
  typename std::list<Key>::iterator iterator_list_;
  typename std::list<Key>::iterator check;

 public:
  Iterator(const typename std::vector<std::list<Key>>::iterator &vector,
           const typename std::vector<std::list<Key>>::iterator &vector_end,
           const typename std::list<Key>::iterator &list)
      : iterator_vector_(vector), iterator_vector_end_(vector_end), iterator_list_(list){};

  Iterator &operator++() {
    if (++iterator_list_ != iterator_vector_->end()) {
      return *this;
    }
    while (++iterator_vector_ != iterator_vector_end_ && iterator_vector_->empty()) {
    }
    if (iterator_vector_ == iterator_vector_end_) {
      iterator_list_ = (--iterator_vector_)->end();
      ++iterator_vector_;
      return *this;
    }
    iterator_list_ = iterator_vector_->begin();
    return *this;
  }

  Iterator operator++(int) {
    auto copy_iterator = *this;
    ++*this;
    return copy_iterator;
  }

  typename std::list<Key>::iterator &operator->() {
    return iterator_list_;
  }

  Key &operator*() {
    return *iterator_list_;
  }

  bool operator!=(const Iterator<Key> &other) {
    return !(*this == other);
  }

  bool operator==(const Iterator<Key> &other) {
    return iterator_list_ == other.iterator_list_;
  }
};

template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class UnorderedSet {
 private:
  std::vector<std::list<Key>> set_{};
  size_t n_bucket_{};
  size_t n_elements_{};
  float load_factor_{};

 public:
  template <class>
  friend class Iterator;

 public:
  using ValueType = Key;
  using SizeType = size_t;
  using Hasher = Hash;
  using Reference = ValueType &;
  using ConstReference = const ValueType &;
  using IteratorSet = Iterator<Key>;

 public:
  UnorderedSet() = default;

  explicit UnorderedSet(size_t count) : n_bucket_(count) {
    set_.resize(count);
  }

  template <class Iterator, class = std::enable_if_t<std::is_base_of_v<
                                std::forward_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>
  UnorderedSet(Iterator first, Iterator second) : n_bucket_(std::distance(first, second)) {
    set_.resize(std::distance(first, second));
    for (auto iter = first; iter != second; ++iter) {
      HashAndPushIfElementIsNotInSet(*iter);
    }
    MakeLoadFactor();
  }

  UnorderedSet(const UnorderedSet &other) = default;

  UnorderedSet(UnorderedSet &&other) noexcept
      : set_(std::move(other.set_))
      , n_bucket_(std::exchange(other.n_bucket_, 0))
      , n_elements_(std::exchange(other.n_elements_, 0))
      , load_factor_(std::exchange(other.load_factor_, 0)){};

  UnorderedSet &operator=(const UnorderedSet &other) {
    if (this != &other) {
      set_ = other.set_;
      n_bucket_ = other.n_bucket_;
      n_elements_ = other.n_elements_;
      load_factor_ = other.load_factor_;
    }
    return *this;
  };

  UnorderedSet &operator=(UnorderedSet &&other) noexcept {
    if (this != &other) {
      set_ = std::move(other.set_);
      n_bucket_ = std::exchange(other.n_bucket_, 0);
      n_elements_ = std::exchange(other.n_elements_, 0);
      load_factor_ = std::exchange(other.load_factor_, 0);
    }
    return *this;
  };

  ~UnorderedSet() noexcept = default;

  inline void MakeLoadFactor() {
    load_factor_ = static_cast<float>(n_elements_) / n_bucket_;
  }

  SizeType HashValue(const ValueType value) const {
    return std::hash<Key>{}(value) % n_bucket_;
  }

  std::pair<IteratorSet, bool> CheckIfElementInSetWithIterator(const size_t idx, const ValueType &value) {
    if (!set_[idx].empty()) {
      auto iter = set_[idx].begin();
      for (auto &item : set_[idx]) {
        if (item == value) {
          return std::make_pair(IteratorSet(set_.begin() + idx, set_.end(), iter), false);
        }
        iter = std::next(iter);
      }
    }
    return std::make_pair(IteratorSet(set_.begin() + idx, set_.end(), set_[idx].begin()), true);
  }

  bool CheckIfElementInSet(const size_t idx, const ValueType &value) const {
    if (idx >= n_bucket_) {
      return false;
    }
    if (!set_[idx].empty()) {
      for (auto &item : set_[idx]) {
        if (item == value) {
          return true;
        }
      }
    }
    return false;
  }

  std::pair<IteratorSet, bool> HashAndPushWithIterator(const ValueType &value) {
    auto idx = HashValue(value);
    set_[idx].push_front(value);
    ++n_elements_;
    MakeLoadFactor();
    return std::make_pair(IteratorSet(set_.begin() + idx, set_.end(), set_[idx].begin()), true);
  }

  void HashAndPush(const ValueType &value) {
    auto idx = HashValue(value);
    set_[idx].push_front(value);
    ++n_elements_;
  }

  bool HashAndPushIfElementIsNotInSet(const ValueType &value) {
    auto idx = HashValue(value);
    if (CheckIfElementInSet(idx, value)) {
      return false;
    }
    set_[idx].push_front(value);
    ++n_elements_;
    return true;
  }

  bool CheckLoadFactor() {
    return load_factor_ < 1;
  }

  [[nodiscard]] SizeType Size() const noexcept {
    return n_elements_;
  }

  [[nodiscard]] bool Empty() const noexcept {
    return n_elements_ == 0;
  }

  IteratorSet end() {  // NOLINT
    return IteratorSet(set_.end(), set_.end(), (--set_.end())->end());
  }

  IteratorSet begin() {  // NOLINT
    auto iter = set_.begin();
    while (iter->empty()) {
      ++iter;
    }
    return IteratorSet(iter, set_.end(), iter->begin());
  }

  void Clear() {
    set_.clear();
    n_elements_ = 0;
    n_bucket_ = 0;
    load_factor_ = 0;
  }

  void Rehash(size_t new_bucket_count) {
    if (new_bucket_count < n_elements_ || new_bucket_count == n_bucket_) {
      return;
    }
    UnorderedSet copy_set = *this;
    this->Clear();
    set_.resize(new_bucket_count);
    n_bucket_ = new_bucket_count;
    if (copy_set.n_bucket_ == 0) {
      return;
    }
    for (auto &item : copy_set) {
      HashAndPush(item);
    }
    MakeLoadFactor();
  }

  [[nodiscard]] bool Find(const ValueType &value) const {
    if (n_bucket_ == 0) {
      return false;
    }
    auto idx = HashValue(value);
    return CheckIfElementInSet(idx, value);
  }

  std::pair<IteratorSet, bool> Insert(const ValueType &insert_value) {
    if (n_bucket_ == 0) {
      ++n_bucket_;
      set_.resize(1);
      return HashAndPushWithIterator(insert_value);
    }
    std::pair<IteratorSet, bool> pair = CheckIfElementInSetWithIterator(HashValue(insert_value), insert_value);
    if (!pair.second) {
      return pair;
    }
    if (CheckLoadFactor()) {
      return HashAndPushWithIterator(insert_value);
    }
    Rehash(2 * n_bucket_);
    return HashAndPushWithIterator(insert_value);
  }

  std::pair<IteratorSet, bool> Insert(ValueType &&insert_value) {
    ValueType copy_insert_value = std::move(insert_value);
    return Insert(copy_insert_value);
  }

  size_t Erase(const ValueType &erase_value) {
    std::pair<IteratorSet, bool> pair = CheckIfElementInSetWithIterator(HashValue(erase_value), erase_value);
    if (pair.second) {
      return 0;
    }
    pair.first.iterator_vector_->erase(pair.first.iterator_list_);
    --n_elements_;
    MakeLoadFactor();
    return 1;
  }

  void Reserve(size_t new_bucket_count) {
    if (new_bucket_count > n_bucket_) {
      Rehash(new_bucket_count);
    }
  }

  [[nodiscard]] SizeType BucketCount() const {
    return n_bucket_;
  }

  [[nodiscard]] size_t BucketSize(size_t id) const {
    if (id >= n_bucket_) {
      return 0;
    }
    return set_[id].size();
  }

  [[nodiscard]] size_t Bucket(ValueType value) const {
    return HashValue(value);
  }

  [[nodiscard]] float LoadFactor() const {
    return load_factor_;
  }
};

#endif  // UNORDERED_SET_UNORDERED_SET_H
