#define VECTOR_MEMORY_IMPLEMENTED

#include <iostream>
#include <numeric>
#include <exception>
#include <type_traits>
#include <memory>
#include <algorithm>
#pragma once

template <typename T>
class Vector {
 private:
  void* buffer_;
  size_t size_;
  size_t capacity_;

  template <typename U>
  friend bool operator<(const Vector<U>& first, const Vector<U>& second);
  template <typename U>
  friend bool operator<=(const Vector<U>& first, const Vector<U>& second);
  template <typename U>
  friend bool operator>(const Vector<U>& first, const Vector<U>& second);
  template <typename U>
  friend bool operator>=(const Vector<U>& first, const Vector<U>& second);
  template <typename U>
  friend bool operator==(const Vector<U>& first, const Vector<U>& second);

  void* Extend() {
    if (capacity_ == 0) {
      auto new_buffer = operator new(sizeof(ValueType));
      return new_buffer;
    }
    auto new_buffer = operator new(capacity_ * 2 * sizeof(ValueType));
    std::uninitialized_move_n(static_cast<Pointer>(buffer_), size_, static_cast<Pointer>(new_buffer));
    return new_buffer;
  }

 public:
  using ValueType = T;
  using Pointer = ValueType*;
  using ConstPointer = const ValueType*;
  using Reference = ValueType&;
  using ConstReference = const ValueType&;
  using SizeType = size_t;
  using Iterator = T*;
  using ConstIterator = const T*;
  using ReverseIterator = std::reverse_iterator<Iterator>;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

  Vector() : buffer_(nullptr), size_(0), capacity_(0) {
  }

  explicit Vector(size_t size) : Vector() {
    if (size == 0) {
      return;
    }
    auto new_buffer = operator new(sizeof(ValueType) * size);
    try {
      std::uninitialized_default_construct_n(static_cast<Pointer>(new_buffer), size);
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    buffer_ = new_buffer;
    size_ = size;
    capacity_ = size;
  }

  Vector(size_t size, ConstReference value) : Vector() {
    if (size == 0) {
      return;
    }
    auto new_buffer = operator new(sizeof(ValueType) * size);
    try {
      std::uninitialized_fill_n(static_cast<Pointer>(new_buffer), size, value);
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    capacity_ = size_ = size;
    buffer_ = new_buffer;
  }

  template <class Iterator, class = std::enable_if_t<std::is_base_of_v<
                                std::forward_iterator_tag, typename std::iterator_traits<Iterator>::iterator_category>>>
  Vector(Iterator first, Iterator second) : Vector() {
    if (first == second) {
      return;
    }
    size_t size = second - first;
    auto new_buffer = operator new(size * sizeof(T));
    try {
      std::uninitialized_copy(first, second, static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    size_ = capacity_ = size;
    buffer_ = new_buffer;
  }

  Vector(const std::initializer_list<T>& list) : Vector() {
    if (list.size() == 0) {
      return;
    }
    size_t size = list.size();
    auto new_buffer = operator new(size * sizeof(ValueType));
    try {
      std::uninitialized_copy(list.begin(), list.end(), static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    size_ = capacity_ = size;
    buffer_ = new_buffer;
  }

  Vector(const Vector<T>& other) : Vector() {
    if (other.size_ == 0) {
      return;
    }
    auto new_buffer = operator new(other.size_ * sizeof(ValueType));
    try {
      std::uninitialized_copy(other.begin(), other.end(), static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    buffer_ = new_buffer;
    size_ = capacity_ = other.size_;
  }

  Vector(Vector<T>&& other) : buffer_(other.buffer_), size_(other.size_), capacity_(other.size_) {
    other.size_ = other.capacity_ = 0;
    other.buffer_ = nullptr;
  }

  Vector& operator=(const Vector<T>& other) {
    if (this == &other) {
      return *this;
    }
    if (capacity_ < other.size_) {
      auto new_buffer = operator new(other.size_ * sizeof(ValueType));
      try {
        std::uninitialized_copy(other.begin(), other.end(), static_cast<Pointer>(new_buffer));
      } catch (...) {
        operator delete(new_buffer);
        throw;
      }
      std::destroy(begin(), end());
      operator delete(buffer_);
      buffer_ = new_buffer;
      size_ = capacity_ = other.size_;
    } else if (size_ > other.size_) {
      std::copy(other.begin(), other.end(), static_cast<Pointer>(buffer_));
      std::destroy(begin() + other.size_, begin() + size_);
      size_ = other.size_;
    } else {
      std::copy(other.begin(), other.begin() + size_, static_cast<Pointer>(buffer_));
      std::uninitialized_copy(other.begin() + size_, other.end(), static_cast<Pointer>(buffer_) + size_);
      size_ = other.size_;
    }
    return *this;
  }

  Vector& operator=(Vector<T>&& other) noexcept {
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = other.buffer_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.buffer_ = nullptr;
    other.size_ = other.capacity_ = 0;
    return *this;
  }

  Vector& operator=(const std::initializer_list<T>& list) {
    if (capacity_ < list.size()) {
      auto new_buffer = operator new(list.size());
      try {
        std::uninitialized_move(list.begin(), list.end(), static_cast<Pointer>(new_buffer));
      } catch (...) {
        operator delete(new_buffer);
        throw;
      }
      buffer_ = new_buffer;
      size_ = capacity_ = list.size();
    } else if (size_ > list.size()) {
      std::copy(list.begin(), list.end(), static_cast<Pointer>(buffer_));
      std::destroy(begin() + list.size(), begin() + size_);
    } else {
      std::move(list.begin(), list.begin() + size_, static_cast<Pointer>(buffer_));
      std::uninitialized_move(list.begin() + size_, list.end(), static_cast<Pointer>(buffer_) + size_);
    }
    return *this;
  }

  ~Vector() noexcept {
    if (buffer_ != nullptr) {
      std::destroy(begin(), end());
      operator delete(buffer_);
      size_ = capacity_ = 0;
    }
  }

  size_t Size() const noexcept {
    return size_;
  }

  size_t Capacity() const noexcept {
    return capacity_;
  }

  bool Empty() const noexcept {
    return size_ == 0;
  }

  Reference operator[](size_t idx) noexcept {
    return *(static_cast<Pointer>(buffer_) + idx);
  }

  ConstReference operator[](size_t idx) const noexcept {
    return *(static_cast<Pointer>(buffer_) + idx);
  }

  Reference At(size_t idx) {
    if (idx >= size_) {
      throw std::out_of_range{"Vector out of range"};
    }
    return static_cast<Pointer>(buffer_)[idx];
    ;
  }

  ConstReference At(size_t idx) const {
    if (idx >= size_) {
      throw std::out_of_range{"Vector out of range"};
    }
    return static_cast<Pointer>(buffer_)[idx];
  }

  Reference Front() noexcept {
    return static_cast<Pointer>(buffer_)[0];
  }
  ConstReference Front() const noexcept {
    return static_cast<Pointer>(buffer_)[0];
  }

  Reference Back() noexcept {
    return static_cast<Pointer>(buffer_)[size_ - 1];
  }
  ConstReference Back() const noexcept {
    return static_cast<Pointer>(buffer_)[size_ - 1];
  };

  Pointer Data() noexcept {
    return static_cast<Pointer>(buffer_);
  }

  ConstPointer Data() const noexcept {
    return static_cast<Pointer>(buffer_);
  }

  void Swap(Vector<T>& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  void Resize(size_t new_size) {
    if (new_size <= capacity_) {
      if (new_size < size_) {
        std::destroy(static_cast<Pointer>(buffer_) + new_size, static_cast<Pointer>(buffer_) + size_);
      } else {
        std::uninitialized_default_construct(static_cast<Pointer>(buffer_) + size_,
                                             static_cast<Pointer>(buffer_) + new_size);
      }
      size_ = new_size;
      return;
    }

    auto new_buffer = operator new(new_size * sizeof(ValueType));
    try {
      std::uninitialized_move(static_cast<Pointer>(buffer_), static_cast<Pointer>(buffer_) + size_,
                              static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
    }
    try {
      std::uninitialized_default_construct(static_cast<Pointer>(new_buffer) + size_,
                                           static_cast<Pointer>(new_buffer) + new_size);
    } catch (...) {
      std::move(static_cast<Pointer>(new_buffer), static_cast<Pointer>(new_buffer) + size_,
                static_cast<Pointer>(buffer_));
      operator delete(new_buffer);
      throw;
    }
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    size_ = new_size;
    capacity_ = new_size;
  }

  void Resize(size_t new_size, const T& value) {
    if (new_size <= capacity_) {
      if (new_size < size_) {
        std::destroy(static_cast<Pointer>(buffer_) + new_size, static_cast<Pointer>(buffer_) + size_);
      } else {
        std::uninitialized_fill(static_cast<Pointer>(buffer_) + size_, static_cast<Pointer>(buffer_) + new_size, value);
      }
      size_ = new_size;
      return;
    }
    auto new_buffer = operator new(new_size * sizeof(ValueType));
    try {
      std::uninitialized_move(static_cast<Pointer>(buffer_), static_cast<Pointer>(buffer_) + size_,
                              static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
    }
    try {
      std::uninitialized_fill(static_cast<Pointer>(new_buffer) + size_, static_cast<Pointer>(new_buffer) + new_size,
                              value);
    } catch (...) {
      std::move(static_cast<Pointer>(new_buffer), static_cast<Pointer>(new_buffer) + size_,
                static_cast<Pointer>(buffer_));
      operator delete(new_buffer);
      throw;
    }
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    size_ = new_size;
    capacity_ = new_size;
  }

  void Reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) {
      return;
    }
    auto new_buffer = operator new(sizeof(ValueType) * new_capacity);
    try {
      std::uninitialized_move(begin(), end(), static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    capacity_ = new_capacity;
  }

  void ShrinkToFit() {
    if (size_ == capacity_) {
      return;
    }

    if (size_ == 0) {
      std::destroy(begin(), end());
      operator delete(buffer_);
      buffer_ = nullptr;
      size_ = capacity_ = 0;
      return;
    }

    auto new_buffer = operator new(sizeof(T) * size_);
    try {
      std::uninitialized_move(begin(), end(), static_cast<Pointer>(new_buffer));
    } catch (...) {
      operator delete(new_buffer);
      throw;
    }
    std::destroy(begin(), end());
    operator delete(buffer_);
    capacity_ = size_;
    buffer_ = new_buffer;
  }

  void Clear() noexcept {
    if (size_ > 0) {
      std::destroy(begin(), end());
      size_ = 0;
    }
  }

  void PushBack(const T& value) {
    if (size_ < capacity_) {
      new (static_cast<Pointer>(buffer_) + size_) T(value);
      size_++;
      return;
    }

    void* new_buffer = nullptr;
    try {
      new_buffer = Extend();
      new (static_cast<Pointer>(new_buffer) + size_) T(value);
    } catch (...) {
      std::destroy_n(static_cast<Pointer>(new_buffer), size_);
      operator delete(new_buffer);
      throw;
    }
    capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    size_++;
  }

  void PushBack(T&& value) {
    if (size_ < capacity_) {
      new (static_cast<Pointer>(buffer_) + size_) T(std::move(value));
      size_++;
      return;
    }

    void* new_buffer = nullptr;
    try {
      new_buffer = Extend();
      new (static_cast<Pointer>(new_buffer) + size_) T(std::move(value));
    } catch (...) {
      std::destroy_n(static_cast<Pointer>(new_buffer), size_);
      operator delete(new_buffer);
      throw;
    }
    capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    size_++;
  }

  template <class... Args>
  void EmplaceBack(Args&&... args) {
    if (size_ < capacity_) {
      new (static_cast<Pointer>(buffer_) + size_) T(std::forward<Args>(args)...);
      size_++;
      return;
    }

    void* new_buffer = nullptr;
    try {
      new_buffer = Extend();
      new (static_cast<Pointer>(new_buffer) + size_) T(std::forward<Args>(args)...);
    } catch (...) {
      std::uninitialized_move(static_cast<Pointer>(new_buffer), static_cast<Pointer>(new_buffer) + size_,
                              static_cast<Pointer>(buffer_));
      operator delete(new_buffer);
      throw;
    }
    capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
    std::destroy(begin(), end());
    operator delete(buffer_);
    buffer_ = new_buffer;
    size_++;
  }

  void PopBack() {
    if (size_ > 0) {
      std::destroy_at(static_cast<Pointer>(buffer_) + size_ - 1);
      size_--;
    }
  }

  Iterator begin() noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_);
  }
  ConstIterator begin() const noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_);
  }

  Iterator end() noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_) + size_;
  }
  ConstIterator end() const noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_) + size_;
  }

  ConstIterator cbegin() const noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_);
  }

  ConstIterator cend() const noexcept {  // NOLINT
    return static_cast<Pointer>(buffer_) + size_;
  }

  ReverseIterator rbegin() noexcept {  // NOLINT
    return std::reverse_iterator(end());
  }
  ConstReverseIterator rbegin() const noexcept {  // NOLINT
    return std::reverse_iterator(cend());
  }
  ReverseIterator rend() noexcept {  // NOLINT
    return std::reverse_iterator(begin());
  }
  ConstReverseIterator rend() const noexcept {  // NOLINT
    return std::reverse_iterator(cbegin());
  }

  ConstReverseIterator crbegin() const noexcept {  // NOLINT
    return std::reverse_iterator(cend());
  }
  ConstReverseIterator crend() const noexcept {  // NOLINT
    return std::reverse_iterator(cbegin());
  }
};

template <typename T>
bool operator<(const Vector<T>& first, const Vector<T>& second) {
  size_t end = std::min(first.size_, second.size_);
  for (size_t idx = 0; idx < end; ++idx) {
    if (first[idx] < second[idx]) {
      return true;
    }
    if (first[idx] > second[idx]) {
      return false;
    }
  }
  return static_cast<bool>(first.size_ < second.size_);
}

template <typename T>
bool operator==(const Vector<T>& first, const Vector<T>& second) {
  if (first.size_ != second.size_) {
    return false;
  }
  for (size_t idx = 0; idx < first.size_; ++idx) {
    if (first[idx] != second[idx]) {
      return false;
    }
  }
  return true;
}

template <typename T>
bool operator>=(const Vector<T>& first, const Vector<T>& second) {
  return !(first < second);
}

template <typename T>
bool operator>(const Vector<T>& first, const Vector<T>& second) {
  return (!(first < second) && !(first == second));
}

template <typename T>
bool operator<=(const Vector<T>& first, const Vector<T>& second) {
  return (first < second || first == second);
}

template <typename T>
bool operator!=(const Vector<T>& first, const Vector<T>& second) {
  return !(first == second);
}
