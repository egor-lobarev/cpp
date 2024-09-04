#include "cppstring.h"
#include <iostream>

size_t String::GetCStringSize(const char* string) const {
  size_t size = 0;
  while (string[size] != '\0') {
    ++size;
  }
  return size;
}

void String::CopyFromCString(const char* string, size_t size) {
  if (capacity_ == 0 && size > 0) {
    string_ = new char[2];
    capacity_ = 2;
  }
  if (size > capacity_) {
    Extend(size / capacity_ + 1);
  }
  for (size_t i = 0; i < size; ++i) {
    string_[i] = string[i];
  }
}

String::String() : string_(nullptr), size_(0), capacity_(0) {
}

String::String(const char* string) : String() {
  capacity_ = size_ = GetCStringSize(string);
  if (size_ != 0) {
    string_ = new char[size_];
  }
  CopyFromCString(string, size_);
}

String::String(const String& other) : String() {
  size_ = other.size_;
  capacity_ = size_;
  if (size_ != 0) {
    string_ = new char[size_];
  }
  CopyFromCString(other.string_, size_);
}

String::String(const size_t size, const char symbol) : String() {
  capacity_ = size_ = size;
  if (size > 0) {
    string_ = new char[size_];
  }
  for (size_t i = 0; i < size_; ++i) {
    string_[i] = symbol;
  }
}

String::String(const char* string, const size_t size) : String() {
  capacity_ = size_ = size;
  if (size_ != 0) {
    string_ = new char[size_];
  }
  CopyFromCString(string, size_);
}

String::~String() {
  delete[] string_;
}

String& String::operator=(const String& other) {
  if (this != &other) {
    CopyFromCString(other.string_, other.size_);
    size_ = capacity_ = other.size_;
  }
  return *this;
}

char& String::operator[](size_t idx) {
  return string_[idx];
}

char String::operator[](size_t idx) const {
  return string_[idx];
}

String operator+(const String& first, const String& second) {
  String new_string;
  for (size_t i = 0; i < first.size_; ++i) {
    new_string.PushBack(first.string_[i]);
  }
  for (size_t i = first.size_; i < first.size_ + second.size_; ++i) {
    new_string.PushBack(second.string_[i - first.size_]);
  }
  return new_string;
}

char String::At(size_t idx) const {
  if (idx >= size_) {
    throw StringOutOfRange{};
  }
  return string_[idx];
}

char& String::At(size_t idx) {
  if (idx >= size_) {
    throw StringOutOfRange{};
  }
  return string_[idx];
}

char& String::Front() {
  return string_[0];
}

char String::Front() const {
  return string_[0];
}

char& String::Back() {
  return string_[size_ - 1];
}

char String::Back() const {
  return string_[size_ - 1];
}

char* String::CStr() {
  return string_;
}

char* String::Data() {
  return string_;
}

const char* String::CStr() const {
  return string_;
}

const char* String::Data() const {
  return string_;
}

bool String::Empty() const {
  return (size_ == 0);
}

void String::Clear() {
  size_ = 0;
}

size_t String::Length() const {
  return size_;
}

size_t String::Size() const {
  return size_;
}

size_t String::Capacity() const {
  return capacity_;
}

void String::Swap(String& other) {
  auto tmp_size = size_;
  auto tmp_capacity = capacity_;
  auto tmp_string = string_;
  string_ = other.string_;
  capacity_ = other.capacity_;
  size_ = other.size_;
  other.size_ = tmp_size;
  other.capacity_ = tmp_capacity;
  other.string_ = tmp_string;
}

void String::PopBack() {
  size_--;
}

void String::ExtendAndCopy(size_t multiply = 2) {
  if (capacity_ == 0) {
    string_ = new char[2];
    capacity_ = 2;
    return;
  }
  capacity_ *= multiply;
  char* old_string = string_;
  string_ = new char[capacity_];

  for (size_t i = 0; i < size_; ++i) {
    string_[i] = old_string[i];
  }
  delete[] old_string;
}

void String::Extend(size_t multiply = 2) {
  capacity_ *= multiply;
  char* old_string = string_;
  string_ = new char[capacity_];
  delete[] old_string;
}

void String::PushBack(char symbol) {
  if (size_ + 1 > capacity_) {
    ExtendAndCopy();
  }
  string_[size_] = symbol;
  size_++;
}

String& String::operator+=(const String& other) {
  for (size_t i = 0; i < other.size_; ++i) {
    PushBack(other.string_[i]);
  }
  return *this;
}

void String::Resize(size_t new_size, char symbol) {
  if (capacity_ == 0) {
    string_ = new char[2];
    capacity_ = 2;
  }
  if (new_size > capacity_) {
    ExtendAndCopy(new_size / capacity_ + 1);
    for (auto i = size_; i < new_size; ++i) {
      string_[i] = symbol;
    }
  }
  size_ = new_size;
}

void String::Reserve(size_t new_capacity) {
  if (capacity_ < new_capacity) {
    ExtendAndCopy(new_capacity / capacity_ + 1);
  }
}

void String::ShrinkToFit() {
  if (size_ == 0) {
    size_ = capacity_ = 0;
    delete[] string_;
    string_ = nullptr;
    return;
  }
  char* old_string = string_;
  string_ = new char[size_];
  capacity_ = size_;
  CopyFromCString(old_string, size_);
  delete[] old_string;
}

bool operator<(const String& first, const String& second) {
  for (size_t i = 0; i < first.size_ && i < second.size_; ++i) {
    if (first.string_[i] < second.string_[i]) {
      return true;
    }
    if (first.string_[i] > second.string_[i]) {
      return false;
    }
  }
  return (first.size_ < second.size_);
}

bool operator==(const String& first, const String& second) {
  if (first.size_ != second.size_) {
    return false;
  }
  for (size_t i = 0; i < first.size_; ++i) {
    if (first.string_[i] != second.string_[i]) {
      return false;
    }
  }
  return true;
}

bool operator>(const String& first, const String& second) {
  for (size_t i = 0; i < first.size_ && i < second.size_; ++i) {
    if (first.string_[i] > second.string_[i]) {
      return true;
    }
    if (first.string_[i] < second.string_[i]) {
      return false;
    }
  }
  return (first.size_ > second.size_);
}

bool operator!=(const String& first, const String& second) {
  return !(first == second);
}

bool operator>=(const String& first, const String& second) {
  return !(first < second);
}

bool operator<=(const String& first, const String& second) {
  return !(first > second);
}

std::ostream& operator<<(std::ostream& os, const String& string) {
  for (size_t i = 0; i < string.size_; ++i) {
    os << string[i];
  }
  return os;
}
