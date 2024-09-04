#include <stdexcept>
#pragma once
#include <iostream>

class StringOutOfRange : public std::out_of_range {
 public:
  StringOutOfRange() : std::out_of_range("StringOutOfRange") {
  }
};

class String {
 public:
  String();
  String(const size_t size, const char symbol);
  String(const char* string);  // NOLINT
  String(const char* string, const size_t size);
  String(const String& other);

  ~String();
  char operator[](size_t idx) const;
  char& operator[](size_t idx);
  String& operator=(const String& other);
  String& operator+=(const String&);

  char At(size_t idx) const;
  char& At(size_t idx);
  char& Front();
  char Front() const;
  char& Back();
  char Back() const;
  const char* CStr() const;
  const char* Data() const;
  char* CStr();
  char* Data();
  bool Empty() const;
  size_t Length() const;
  size_t Size() const;
  size_t Capacity() const;
  void Clear();
  void Swap(String& other);
  void PopBack();
  void PushBack(char symbol);
  void Resize(size_t new_size, char symbol);
  void Reserve(size_t new_capacity);
  void ShrinkToFit();

 private:
  char* string_;
  size_t size_;
  size_t capacity_;

  size_t GetCStringSize(const char* string) const;
  void CopyFromCString(const char* string, size_t size);
  void Extend(size_t multiply);
  void ExtendAndCopy(size_t multiply);
  friend String operator+(const String& first, const String& second);
  friend bool operator<(const String& first, const String& second);
  friend bool operator==(const String& first, const String& second);
  friend bool operator>(const String& first, const String& second);
  friend bool operator!=(const String& first, const String& second);
  friend bool operator>=(const String& first, const String& second);
  friend bool operator<=(const String& first, const String& second);
  friend std::ostream& operator<<(std::ostream& os, const String& string);
};

String operator+(const String& first, const String& second);
bool operator<(const String& first, const String& second);
bool operator==(const String& first, const String& second);
bool operator>(const String& first, const String& second);
bool operator!=(const String& first, const String& second);
bool operator>=(const String& first, const String& second);
bool operator<=(const String& first, const String& second);
std::ostream& operator<<(std::ostream& os, const String& string);
